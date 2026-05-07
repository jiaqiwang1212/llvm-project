# 'linalg' 方言

[TOC]

## 设计原理

<img width="90" align="left" alt="MLIR Codegen Flow" src="https://user-images.githubusercontent.com/10148468/73613629-c5586580-45c5-11ea-94b7-074aeea94c7b.png">

Linalg 旨在解决 MLIR 中的高层次分层优化（HHO box）问题，并能在*混合专家编译器*环境（即 *CGSel* box）中良好地互操作。

[设计原理文档](../../Rationale/RationaleLinalgDialect.md)中包含了更多设计和架构决策的详细说明。

## 关键变换集合<a name="key_transformations"></a>

以下关键变换是驱动 Linalg 设计的核心。它们全部基于 `linalg.generic` OpInterface 的属性来实现，避免了依赖硬编码的一次性操作知识的陷阱。

这些变换的文字形式描述留待未来工作完成。尽管如此，列出在 Linalg IR 上执行的关键变换（它们已影响到 Linalg 的设计）仍然是有意义的：

1.  渐进式缓冲区分配（Progressive Buffer Allocation）。
1.  参数化分块（Parametric Tiling）。
1.  提升到快速内存中的临时缓冲区（Promotion to Temporary Buffer in Fast Memory）。
1.  带参数化分块融合的分块生产者-消费者融合（Tiled Producer-Consumer Fusion with Parametric Tile-And-Fuse）。
1.  映射到并行与归约循环及硬件（Map to Parallel and Reduction Loops and Hardware）。
1.  向量化：以向量形式重写（Vectorization: Rewrite in Vector Form）。
1.  降低到循环（仿射、通用和并行）。
1.  降低到库调用或特殊指令、内联函数或 ISA。
1.  部分降低到更细粒度 Linalg 操作上的迭代。

## Linalg 操作的高层描述<a name="linalg_ops"></a>

Linalg 至少从所有之前[列出的先行工作](../../Rationale/RationaleLinalgDialect.md/#prior-art)中汲取了一些灵感。该设计支持定义具有通用属性的 ***CustomOps***，这些属性支持[关键变换](#key_transformations)，包括降低到标量 load/store 和其他操作，或降低到外部库调用和内联函数。

这些操作可以将***张量或缓冲区***作为输入和输出操作数。输出张量操作数的目的是提供统一的抽象并为结果赋予形状。输出张量有两种形式，始终与相应的操作结果关联：

1.  "init tensor"输出值，为通过迭代更新结果创建的张量提供初始值（也称为"破坏性更新"）。此类张量总会以某种形式具体化。如果发生足够多的融合，它最终可能只以寄存器级 SSA 值的形式具体化。预期（但不要求）破坏性更新模式可以被重写为对缓冲区的原地更新。

2.  "仅形状"张量输出值，其底层元素不在载荷计算中使用，仅用于向更低抽象层次传递形状信息。未来当合适的形状类型作为内置类型可用时，这将被替换（详见话题讨论 [Linalg and Shapes](https://llvm.discourse.group/t/linalg-and-shapes/2421)）。

### 载荷携带操作<a name="payload_ops"></a>

Linalg 定义了一种载荷携带操作，它在张量和缓冲区上实现[结构化操作](https://docs.google.com/presentation/d/1P-j1GrH6Q5gLBjao0afQ-GfvcAeF-QU4GXXeSy0eJ9I/edit#slide=id.p)抽象。这个 `linalg.generic` 操作可以表达自定义操作，这些操作可以选择性地具有*索引语义*（通过 `linalg.index` 操作访问迭代索引）。`linalg.generic` 的属性是将[设计原理文档](../../Rationale/RationaleLinalgDialect.md)中描述的指导原则应用的结果。下面将依次列出这些属性，并对每个属性进行简短的示例和讨论。

#### 属性 1：输入和输出操作数定义迭代空间<a name="prop1"></a>

`linalg.generic` 操作完全*从其操作数派生*其迭代空间的规范。该属性要求局部化的 IR 元素（操作）*拥有*合成控制流所需的所有信息，以便根据操作数的类型对其进行迭代。这种 IR 局部化的概念与 [URUK](http://icps.u-strasbg.fr/~bastoul/research/papers/GVBCPST06-IJPP.pdf) 有些相似。

考虑以下完整规范的 `linalg.generic` 示例。这里第一个操作数是 `f32` 标量元素的 `memref`，具有普通的恒等布局；第二个是 4 元素向量的 `memref`，具有 2 步长、1 偏移的布局。

```mlir
// File name: example1.mlir
#accesses = [
  affine_map<(m) -> (m)>,
  affine_map<(m) -> (m)>
]

#attrs = {
  indexing_maps = #accesses,
  iterator_types = ["parallel"]
}

func.func @example(%A: memref<?xf32, strided<[1]>>,
              %B: memref<?xvector<4xf32>, strided<[2], offset: 1>>) {
  linalg.generic #attrs
  ins(%A: memref<?xf32, strided<[1]>>)
  outs(%B: memref<?xvector<4xf32>, strided<[2], offset: 1>>) {
  ^bb0(%a: f32, %b: vector<4xf32>):
    %c = "some_compute"(%a, %b): (f32, vector<4xf32>) -> (vector<4xf32>)
    linalg.yield %c: vector<4xf32>
  }
  return
}
```

属性"*输入和输出操作数定义迭代空间*"通过降低到如下形式的代码来体现：

```mlir
// Run: mlir-opt example1.mlir -allow-unregistered-dialect -convert-linalg-to-loops
// This converted representation is in the `scf` dialect.
// It's syntax can be found here: https://mlir.llvm.org/docs/Dialects/SCFDialect/

func.func @example(%arg0: memref<?xf32>,
                   %arg1: memref<?xvector<4xf32>, strided<[2], offset: 1>>) {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %0 = memref.dim %arg0, %c0 : memref<?xf32>
  scf.for %arg2 = %c0 to %0 step %c1 {
    %1 = memref.load %arg0[%arg2] : memref<?xf32>
    %2 = memref.load %arg1[%arg2]
       : memref<?xvector<4xf32>, strided<[2], offset: 1>>
    %3 = "some_compute"(%1, %2) : (f32, vector<4xf32>) -> vector<4xf32>
    memref.store %3, %arg1[%arg2]
       : memref<?xvector<4xf32>, strided<[2], offset: 1>>
  }
  return
}
```

该属性有助于简化分析和变换。例如，它从结构上保证不会发生越界访问（假设动态操作数维度彼此一致，这正是 `assert` 运行时检查的目的）。

在降低到循环形式之前，循环归纳变量和迭代器是隐式的（即*尚未具体化*）。

主要含义是：

1.  操作的语义*仅限于对结构化数据类型进行操作*，我们可以在这些类型上定义迭代器。

2.  这不能对具有副作用的任意代码建模。

我们认为这些在实践中并非严重的限制，因为 MLIR 的核心在于在同一 IR 中混合不同层次的抽象。只要 Linalg 能够逐步降低到下一个抽象层次，对于不适合的内容，它也可以被绕过。

同时，将操作语义置于结构化数据类型的条件下，是向非密集张量扩展的一个非常有前景的路径，LIFT 对[稀疏](https://www.lift-project.org/publications/2016/harries16sparse.pdf)和[位置相关数组](https://www.lift-project.org/publications/2019/pizzuti19positiondependentarrays.pdf)的抽象实践，以及 [TACO](http://tensor-compiler.org/) 的经验都证明了这一点。

#### 属性 2：控制结构与数据结构之间的可逆映射<a name="prop2"></a>

`linalg.generic` *定义*了迭代空间（即循环）与数据之间的映射。

考虑以下完整规范的 `linalg.generic` 示例。这里第一个 `memref` 在其两个维度上都是 2 步长的，第二个 `memref` 使用恒等布局。

```mlir
// File name: example2.mlir
#indexing_maps = [
  affine_map<(i, j) -> (j, i)>,
  affine_map<(i, j) -> (j)>
]

#attrs = {
  indexing_maps = #indexing_maps,
  iterator_types = ["parallel", "parallel"]
}

func.func @example(%A: memref<8x?xf32, strided<[2, 2], offset: 0>>,
              %B: memref<?xvector<4xf32>>) {
  linalg.generic #attrs
  ins(%A: memref<8x?xf32, strided<[2, 2], offset: 0>>)
  outs(%B: memref<?xvector<4xf32>>) {
  ^bb0(%a: f32, %b: vector<4xf32>):
    %c = "some_compute"(%a, %b): (f32, vector<4xf32>) -> (vector<4xf32>)
    linalg.yield %c: vector<4xf32>
  }
  return
}
```

属性"*控制结构与数据结构之间的可逆映射*"通过降低到如下形式的代码来体现：

```mlir
// Run: mlir-opt example2.mlir -allow-unregistered-dialect -convert-linalg-to-loops

func.func @example(%arg0: memref<8x?xf32, strided<[2, 2]>>, %arg1: memref<?xvector<4xf32>>) {
  %c8 = arith.constant 8 : index
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %0 = memref.dim %arg0, %c1 : memref<8x?xf32, strided<[2, 2]>>
  scf.for %arg2 = %c0 to %0 step %c1 {
    scf.for %arg3 = %c0 to %c8 step %c1 {
      %1 = memref.load %arg0[%arg3, %arg2] : memref<8x?xf32, strided<[2, 2]>>
      %2 = memref.load %arg1[%arg3] : memref<?xvector<4xf32>>
      %3 = "some_compute"(%1, %2) : (f32, vector<4xf32>) -> vector<4xf32>
      memref.store %3, %arg1[%arg3] : memref<?xvector<4xf32>>
    }
  }
  return
}
```

这个映射需要是可逆的，因为我们希望能够在两者之间来回转换，并回答以下问题：

-   给定迭代空间的一个子集，它读取和写入哪些数据子集？
-   给定读取或写入的数据子集，哪个迭代空间子集负责这个读写？

回答这 `2` 个问题是 Linalg 用于实现分块、分块生产者-消费者融合，以及提升到快速内存中临时缓冲区等变换的主要分析之一。

在当前实现中，`linalg.generic` 使用一组 [AffineMaps](https://mlir.llvm.org/docs/LangRef/#affinemap-attribute)（见前面示例中的 `#indexing_maps` 属性）。这是一个务实的短期解决方案，但从长远来看，注意到这个属性甚至可以被动态评估，类似于 inspector-executor 算法。

#### 属性 3：迭代器的类型被明确定义<a name="prop3"></a>

`linalg.generic` 操作完全*声明*其迭代器的类型。这一信息在变换中会被用到。

这些属性来源于该领域的既有实践，并反映了 Ken Kennedy 的[面向现代架构的优化编译器](https://www.elsevier.com/books/optimizing-compilers-for-modern-architectures/allen/978-0-08-051324-9)中的属性。Kennedy 表达的循环变换合法性的关键思想是：***所有依赖向量的字典序必须被保留***。

这可以通过特定的迭代器类型在循环层面得到更好地捕获，包括：*并行*、*归约*、*划分*、*可置换/单调*、*顺序*、*依赖距离*等。

这些类型传统上是复杂依赖分析的结果，在多面体社区中被称为"*带*"（例如，[ISL](https://en.wikipedia.org/wiki/Integer_set_library) 调度树术语中的*并行带*、*可置换带*等）。

在 `linalg.generic` 中以声明方式指定这些信息，允许传递可能难以（甚至不可能）从低层信息推导的属性。这些属性可以一直保留到它们对变换有用的时刻，被使用后再丢弃。

此外，这些属性也可以被视为前端/用户保证的契约，编译器可以利用这些契约。一个常见的例子是使用数据依赖的归约语义来指定直方图计算。如果前端有额外的知识知道合适的原子操作可用，则最好指定并行语义并在计算区域使用特殊的原子操作。

目前，Linalg 只对*并行*和*归约*循环有明确的用途，但先前的经验表明该抽象是可以泛化的。

#### 属性 4：计算载荷通过区域指定<a name="prop4"></a>

`linalg.generic` 操作具有完全通用的计算载荷，这得益于[区域（Regions）](https://github.com/llvm/llvm-project/blob/58265ad42a90ae8905be6a447cb42e53529a54a0/mlir/docs/LangRef.md/#regions)的使用。

区域以 `linalg.generic` 的张量或缓冲区操作数的标量元素类型作为参数。为了灵活性和与库调用的匹配能力，可以传递额外的特殊值。例如，`linalg.fill` 操作接受一个缓冲区和一个额外的标量值。

目前对区域语义没有额外的限制。这是为了允许探索区域与迭代器类型交叉点上的各种设计权衡。特别是，前端负责确保迭代器类型的语义与区域内的操作相对应：区域可以任意捕获缓冲区并向其写入。如果这与某些并行迭代器要求冲突，则属于未定义行为。

前面的示例已经用未注册的函数 `"some_compute"` 阐明了计算载荷。以下代码片段展示了使用具体操作 `addf` 时的结果：

```mlir
// File name: example3.mlir
#map = affine_map<(i, j) -> (i, j)>

#attrs = {
  indexing_maps = [#map, #map, #map],
  iterator_types = ["parallel", "parallel"]
}

func.func @example(%A: memref<?x?xf32>, %B: memref<?x?xf32>, %C: memref<?x?xf32>) {
  linalg.generic #attrs
  ins(%A, %B: memref<?x?xf32>, memref<?x?xf32>)
  outs(%C: memref<?x?xf32>) {
    ^bb0(%a: f32, %b: f32, %c: f32):
      %d = arith.addf %a, %b : f32
      linalg.yield %d : f32
  }

  return
}
```

此函数基本上是对两个矩阵（`%A` 和 `%B`）进行逐元素相加，并将结果存储到另一个矩阵（`%C`）中。

属性"*计算载荷通过区域指定*"通过降低到如下形式的代码来体现：

```mlir
func.func @example(%arg0: memref<?x?xf32>, %arg1: memref<?x?xf32>, %arg2: memref<?x?xf32>) {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %0 = memref.dim %arg0, %c0 : memref<?x?xf32>
  %1 = memref.dim %arg0, %c1 : memref<?x?xf32>
  scf.for %arg3 = %c0 to %0 step %c1 {
    scf.for %arg4 = %c0 to %1 step %c1 {
      %2 = memref.load %arg0[%arg3, %arg4] : memref<?x?xf32>
      %3 = memref.load %arg1[%arg3, %arg4] : memref<?x?xf32>
      %4 = arith.addf %2, %3 : f32
      memref.store %4, %arg2[%arg3, %arg4] : memref<?x?xf32>
    }
  }
  return
}
```

在降低到循环和更低层次构造的过程中，会遇到类似的要求，正如[内联调用操作提案](https://llvm.discourse.group/t/introduce-std-inlined-call-op-proposal/282/2)中所讨论的。我们期望能够复用通用的低层基础设施，前提是它能演化为同时支持区域参数和捕获。

#### 属性 5：可以映射到外部库调用<a name="prop5"></a>

`linalg.generic` 操作可以通过指定 `SymbolAttr` 来映射到外部库调用。在这个抽象层次上，重要的粘合剂是执行变换的能力，这些变换***在应用了不同变换后仍能调用外部库***。

这涉及到操作语义保留和 ABI 层面集成的相关考虑。无论是想使用外部库调用还是自定义 ISA，代码生成所面临的问题是类似的：保持固定的粒度。

考虑以下示例，它添加了一个额外的属性 `library_call="pointwise_add"`，指定了我们打算使用的外部库调用的名称：

```mlir
// File name: example4.mlir
#indexing_maps = [
  affine_map<(i, j) -> (i, j)>,
  affine_map<(i, j) -> (i, j)>,
  affine_map<(i, j) -> (i, j)>
]

#attrs = {
  indexing_maps = #indexing_maps,
  iterator_types = ["parallel", "parallel"],
  library_call = "pointwise_add"
}

func.func @example(%A: memref<?x?xf32>, %B: memref<?x?xf32>, %C: memref<?x?xf32>) {
  linalg.generic #attrs
  ins(%A, %B: memref<?x?xf32>, memref<?x?xf32>)
  outs(%C: memref<?x?xf32>) {
  ^bb0(%a: f32, %b: f32, %c: f32):
    %d = arith.addf %a, %b : f32
    linalg.yield %d : f32
  }
  return
}
```

属性"*映射到外部库调用*"通过降低到如下形式的代码来体现：

```mlir
// Run: mlir-opt example4.mlir -convert-linalg-to-std

func.func @example(%arg0: memref<?x?xf32>, %arg1: memref<?x?xf32>, %arg2: memref<?x?xf32>) {
  %0 = memref.cast %arg0 : memref<?x?xf32> to memref<?x?xf32, strided<[?, ?], offset: ?>>
  %1 = memref.cast %arg1 : memref<?x?xf32> to memref<?x?xf32, strided<[?, ?], offset: ?>>
  %2 = memref.cast %arg2 : memref<?x?xf32> to memref<?x?xf32, strided<[?, ?], offset: ?>>
  call @pointwise_add(%0, %1, %2) : (memref<?x?xf32, strided<[?, ?], offset: ?>>,
    memref<?x?xf32, strided<[?, ?], offset: ?>>, memref<?x?xf32, strided<[?, ?], offset: ?>>) -> ()
  return
}
func.func @pointwise_add(memref<?x?xf32, strided<[?, ?], offset: ?>>,
                         memref<?x?xf32, strided<[?, ?], offset: ?>>,
                         memref<?x?xf32, strided<[?, ?], offset: ?>>) attributes {llvm.emit_c_interface}
```

降低到 LLVM 后类似于：

```mlir
// Run: mlir-opt example4.mlir -convert-linalg-to-std | mlir-opt -convert-func-to-llvm
// Some generated code are omitted here.
func.func @example(%arg0: !llvm<"float*">, ...) {
  ...
  llvm.call @pointwise_add(...) : (!llvm<"float*">, ...) -> ()
  return
}

llvm.func @pointwise_add(%arg0: !llvm<"float*">, ...) attributes {llvm.emit_c_interface} {
  ...
  llvm.call @_mlir_ciface_pointwise_add(%9, %19, %29) : (!llvm."{ float*, float*, i64, [2 x i64], [2 x i64] }*">, !llvm<"{ f32*, f32*, i64, [2 x i64], [2 x i64] }*">, !llvm<"{ float*, float*, i64, [2 x i64], [2 x i64] }
*">) -> ()
  llvm.return
}
llvm.func @_mlir_ciface_pointwise_add(!llvm."{ float*, float*, i64, [2 x i64], [2 x i64] }*">, !llvm<"{ f32*, f32*, i64, [2 x i64], [2 x i64] }*">, !llvm<"{ f32*, f32*, i64, [2 x i64], [2 x i64] }*">) attributes {llvm.emit_c_interface}
```

##### 外部库互操作性的约定

`linalg` 方言采用了与 `BLAS` 类似的约定，当将操作卸载到快速库实现时：传递指向输入和输出数据的非拥有指针以及附加元数据。这一约定也在 `MKL`、`OpenBLAS`、`BLIS`、`cuBLAS`、`cuDNN` 等库中以及跨语言边界的接口点上（例如 C++ / Python）被普遍采用。

通常，`linalg` 将指向 View 数据结构的非拥有指针传递给外部链接的预编译库调用。

关于在存在关键属性时扩展互操作性的话题，目前有一个[持续中的讨论](https://llvm.discourse.group/t/lowering-optional-attributes-in-linalg-structuredops-to-standard-dialect/333/3)。

#### 属性 6：对整个输出操作数进行完美嵌套写入<a name="prop6"></a>

完美嵌套循环是一类特别重要的结构，它支持关键的循环变换，例如分块和映射到库调用。不幸的是，这种结构很容易被部分循环融合等变换破坏，从而使分块和映射到库调用变得更加困难，甚至不可行。Linalg 操作将完美嵌套性作为一等属性：这个结构不能被破坏，并在 IR 中通过构造传递。

`linalg.generic` 操作表示一个写入整个内存区域的完美嵌套循环巢。这是一个跨越区域和循环的结构约束，已被证明是简化变换的关键。

值得一提的是，将不完美嵌套代码转换为完美嵌套代码，通常可以通过足够的循环分发和将条件嵌入到最内层循环来完成。

先前对 Tensor Comprehensions 的经验让我们直觉上感到，强制最内层控制流嵌套很像是用布尔值数组和谓词编写数据并行代码。这种技巧以前也曾在多面体编译器中用于将非仿射控制转换为仿射计算依赖。

虽然从通用 IR 自动化此类重写可能是可行的，但 `linalg.generic` 目前仅强制执行这些语义。

关键含义是，一旦 Linalg 变换完成，就需要撤销这种深谓词转换。在迭代器和归纳变量具体化之后（即在 `linalg.generic` 降低完成之后），整体性能将在很大程度上受到规范化、折叠和*循环不变代码外提*（LICM）质量的影响。

在更宏观的方案中，依赖后期 LICM 被认为是一个必要的风险。

#### 综合总结<a name="summary"></a>

就目前而言，上述六个属性定义了 `linalg.generic` 操作的语义。这是一个悬而未决的问题：在实践中是否所有这些语义都是严格必要的，以及其中一些是否应该或可以在保持[核心指导原则](../../Rationale/RationaleLinalgDialect.md/#core-guiding-principlesa-nameguiding_principlesa)的同时自动推导出来。

目前，我们基于在多个高层编译器上构建和工作的经验确定了这些属性的组合。随着我们将这些内容整理出来并与社区更多互动，我们预期会有多轮讨论和对原始架构的设计变更。

### 数据表示：视图<a name="views"></a>

当前实现使用[步幅 MemRef（又称 View）](https://groups.google.com/a/tensorflow.org/forum/#!topic/mlir/MaL8m2nXuio)抽象。名称 *View* 在 `linalg` 中与 *Strided MemRef* 互换使用。未来我们期望使用其他结构化数据类型，并支持锯齿形、混合稀疏和其他类型。我们期望借鉴现有 LIFT 抽象在[稀疏](https://www.lift-project.org/publications/2016/harries16sparse.pdf)和[位置相关数组](https://www.lift-project.org/publications/2019/pizzuti19positiondependentarrays.pdf)方面的经验。

### 元数据操作<a name="metadata_ops"></a>

一组操作元数据但不移动内存的操作。这些操作接受 `view` 操作数和额外属性，并返回新的 `view`。返回的 `view` 通常是操作数 `view` 的别名。目前现有操作如下：

```
* `memref.view`,
* `memref.subview`,
* `memref.transpose`.
* `linalg.slice`,
* `linalg.reshape`,
```

未来的操作按需添加，但应包括：

```
* `linalg.tile`,
* `linalg.intersection`,
* `linalg.convex_union`,
* `linalg.difference` (would need to work on a list of views).
```

这些附加操作对应于在大规模分布式模板计算领域中已知可行的抽象。

在更长远的未来，来自 [Legion 以数据为中心的编程模型](https://legion.stanford.edu/overview/)的抽象看起来普遍具有吸引力。

### 命名的载荷携带操作<a name="named_ops"></a>

此外，`linalg` 提供了一小部分常用的命名操作：

```
* `linalg.fill`,
* `linalg.dot`,
* `linalg.matmul`,
* `linalg.conv`.
```

这些命名操作遵循 `linalg.generic` 操作接口。目前正在进行工作，以定义声明式机制来自动从仅使用通用操作接口的描述生成命名操作。

这就是为什么今天只有少数操作的主要原因：我们期望它们很快就能从 Tablegen 自动生成。

### 命名载荷操作规范

Linalg 提供了一个声明式规范和生成工具（`mlir-linalg-ods-gen`），可以从受爱因斯坦记法启发的符号中自动生成命名操作。

`mlir-linalg-ods-gen` 中使用的语法和语义正处于快速演变之中，借鉴了 Tensor Comprehensions (TC) 的一些做法，但在几个维度上有所不同，以更好地适应 Linalg：

1.  输入和输出张量参数被指定为 `id : type(symbolic-affine-expression-list)`（例如 `A : f32(M, N + M)`），每个新符号都被贪婪地发现。TC 不允许通用的符号仿射表达式。
1.  输出形状被显式指定，而在 TC 中它们总是从输入形状推导出来。
1.  用于指定计算的操作使用 EDSC 内联函数，以便可以轻松地解析并发射到简单的区域构建器中，而无需诉诸更通用的 MLIR 解析。
1.  归约维度通过操作上的尖括号记法指定（例如 `std_add<k>` 表示 `k` 是一个归约维度）。在 TC 中，归约维度是推断出来的。如果某个操作数未在任何表达式中使用，它将被视为仅形状操作数，indexing_map 的结果将是归约维度。
1.  并行和归约维度按文本程序顺序排列。例如，在综合式 `O(i, j) = std_add<k, l>(...)` 中，`i`（分别为 `j`）是由位置 `0`（分别为 `1`）的仿射维度编码的并行迭代器；`k`（分别为 `l`）是由位置 `2`（分别为 `3`）的仿射维度编码的归约迭代器。
1.  可以为操作定义属性列表，格式为 `attr(strides: 2xi32)`，并在综合式中像 `strides[0]` 一样引用。这些属性使用将被解析为仿射符号以生成操作定义和实现。对于具体的操作实例，来自属性的运行时常量值将被用来替换仿射符号并简化索引映射。

这些决策和语法仍在演变和变化中。特别是，未来可能会添加特定于操作的属性、动态秩、某种形式的模板化、形状计算函数规范等。

目前，对语法和语义施加了以下限制：

1.  每个 def 只能包含一个综合式，但每个综合式可以执行多次更新。
2.  每个张量只能使用一个索引表达式。

可以为命名操作附加一个用 `"""` 包裹的文档字符串。它应该首先包含一行摘要，其后是详细描述。

以下规范可用于定义名为 `batchmatmul` 的操作：

```
def batchmatmul(A: f32(Batch, M, K), B: f32(K, N)) -> (C: f32(Batch, M, N))
"""Batch matrix-multiply operation.

This operation performs batch matrix-multiply over ...
"""
{
  C(b, m, n) = std_addf<k>(std_mulf(A(b, m, k), B(k, n)));
}
```

当调用 `mlir-linalg-ods-gen -gen-ods-decl=1` 时，会生成以下 ODS：

```
def batchmatmulOp : LinalgNamedStructured_Op<"batchmatmul", [
  NInputs<2>,
  NOutputs<1>,
  NamedStructuredOpTrait]> { ... }
```

当调用 `mlir-linalg-ods-gen -gen-impl=1` 时，会生成以下 C++：

```
std::optional<SmallVector<StringRef, 8>> batchmatmul::referenceIterators() {
  return SmallVector<StringRef, 8>{
    getParallelIteratorTypeName(),
    getParallelIteratorTypeName(),
    getParallelIteratorTypeName(),
    getReductionIteratorTypeName() };
}
std::optional<SmallVector<AffineMap, 8>> batchmatmul::referenceIndexingMaps() {
  MLIRContext *context = getContext();
  AffineExpr d0, d1, d2, d3;
  bindDims(context, d0, d1, d2, d3);
  return SmallVector<AffineMap, 8>{
      AffineMap::get(4, 0, {d0, d1, d3}),
      AffineMap::get(4, 0, {d3, d2}),
      AffineMap::get(4, 0, {d0, d1, d2}) };
}
void batchmatmul::regionBuilder(ArrayRef<BlockArgument> args) {
  using namespace edsc;
  using namespace intrinsics;
  Value _0(args[0]), _1(args[1]), _2(args[2]);
  Value _4 = std_mulf(_0, _1);
  Value _5 = std_addf(_2, _4);
  (linalg_yield(ValueRange{ _5 }));
}
```

### 基于 YAML 的命名结构化操作<a name="yaml-gen"></a>

Linalg 提供了一个声明式生成工具（`mlir-linalg-ods-yaml-gen`），用于从旨在捕获命名操作结构的基于 YAML 的操作描述格式中自动生成命名操作。基于 YAML 的操作描述由更高层次的 [DSL](OpDSL.md) 生成，不应直接编辑。

该工具目前仍在开发中，旨在就绪后取代上述工具。有关模式的真实来源，请参阅 `mlir-linalg-ods-yaml-gen.cpp` 中的 C++ 类到 YAML 映射特性。

上述大部分文档大致适用于此路径，并将随着迁移的推进而被移植。

## 开放问题和设计替代方案<a name="open_issues"></a>

目前有多个开放问题和设计替代方案正在讨论中，现在是时候将它们列出来供社区讨论和分析了：

1.  `linalg.generic` 是否应该支持嵌套？
1.  `linalg.generic` 的区域是否应该接受视图还是只接受标量？
1.  我们是否应该尝试在这个抽象层次上解决自动微分问题？
1.  这六个属性真的都是必要的吗？
1.  这是否过于依赖声明式规范，我们是否应该更多地依赖分析？
1.  这对社区的需求来说是否足够通用？如果不够，应该如何扩展（如果有的话）？……

这些关键问题（以及更多）应该在 MLIR 的整体背景下真正思考，在 MLIR 中不同层次的 IR 可以无缝互操作。在实践中，没有必要（也没有益处）尝试在同一 IR 中解决所有问题。

## 操作

[include "Dialects/LinalgOps.md"]
