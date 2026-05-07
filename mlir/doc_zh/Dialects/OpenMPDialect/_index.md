# 'omp' 方言

`omp` 方言用于表示 [OpenMP 编程模型](https://www.openmp.org)的指令、子句和其他定义。这种基于指令的编程模型为 C、C++ 和 Fortran 编程语言定义，提供了简化并行和加速程序开发的抽象。OpenMP 规范的所有版本可以在[此处](https://www.openmp.org/specifications/)找到。

该 MLIR 方言中的操作通常对应于单个 OpenMP 指令，接受表示其支持子句的参数，但并非总是如此。关于该方言中操作、类型和其他定义的详细信息，请参阅自动生成的 [ODS 文档](ODS.md)。

[TOC]

## 操作命名约定

本节旨在标准化方言操作名称的选择方式，以确保一定程度的一致性。名称分为两类：tablegen 名称和汇编名称。前者也对应为操作生成的 C++ 类，而后者用于在 MLIR 文本形式中表示它。

Tablegen 名称采用 CamelCase（首字母大写，带有"Op"后缀），而汇编名称采用 snake_case（全小写，单词之间用下划线分隔）。

如果操作对应于 OpenMP 规范中的指令、子句或其他类型的定义，则必须使用相同的名称，以相同的方式拆分成词。例如，`target data` 指令将变成 `TargetDataOp` / `omp.target_data`，而 `taskloop` 将变成 `TaskloopWrapperOp` / `omp.taskloop.wrapper`。

旨在为另一个特定操作或子句携带额外信息的操作必须以该其他操作或子句的名称命名，后跟附加信息的名称。汇编名称必须使用句点分隔两部分。例如，用于定义某些额外映射信息的操作命名为 `MapInfoOp` / `omp.map.info`。如果为同一指令的不同变体创建多个操作，也遵循相同的规则，例如 `atomic` 变成 `Atomic{Read,Write,Update,Capture}Op` / `omp.atomic.{read,write,update,capture}`。

## 基于子句的操作定义

OpenMP 规范的一个主要特点是，即使可以应用于给定指令的子句集与其他指令无关，这些子句通常可以应用于多个指令。由于子句通常定义了相应 MLIR 操作接受的参数，因此可以（且更推荐）基于对应指令所接受的子句列表来定义 OpenMP 方言操作。这使得保持它们在不同操作间的一致表示更加简单，并最小化方言中的冗余。

为此，创建了基础 `OpenMP_Clause` tablegen 类。它旨在用于创建可以附加到多个 `OpenMP_Op` 定义的子句定义，从而使后者默认继承由子句定义的所有属性，类似于 trait 机制。该机制在 [OpenMPOpBase.td](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/OpenMP/OpenMPOpBase.td) 中实现。

### 添加子句

OpenMP 子句定义位于 [OpenMPClauses.td](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/OpenMP/OpenMPClauses.td)。对于每个子句，必须创建一个 `OpenMP_Clause` 子类和一个基于它的定义。子类必须为它可以在关联的 `OpenMP_Op` 上填充的每个属性接受一个 `bit` 模板参数。这些必须转发给基类。定义必须是基类的实例化，其中所有这些模板参数都设置为 `false`。定义的名称必须是 `OpenMP_<Name>Clause`，而其基类的名称必须是 `OpenMP_<Name>ClauseSkip`。遵循此模式使定义操作时可以选择性地跳过某些属性的继承：[更多信息](#overriding-clause-inherited-properties)。

子句可以定义以下属性：
  - `list<Traits> traits`：当具有某个子句始终意味着某个操作 trait 时使用，例如 `map` 子句和 `MapClauseOwningInterface`。
  - `dag(ins) arguments`：必填属性，保存用于表示子句的值和属性。参数名称使用 snake_case，应包含子句名称以避免子句间的名称冲突。可变参数（非属性）必须包含"_vars"后缀。
  - `string {req,opt}AssemblyFormat`：可选格式字符串，用于为与子句关联的参数生成自定义的人性化打印器和解析器。它将与其他子句的汇编格式组合，如[下文](#adding-an-operation)所述。
  - `string description`：可选描述文本，用于描述子句及其表示。
  - `string extraClassDeclaration`：可选 C++ 声明，将添加到包含子句的操作类中。

例如：

```tablegen
class OpenMP_ExampleClauseSkip<
    bit traits = false, bit arguments = false, bit assemblyFormat = false,
    bit description = false, bit extraClassDeclaration = false
  > : OpenMP_Clause<traits, arguments, assemblyFormat, description,
                    extraClassDeclaration> {
  let arguments = (ins
    Optional<AnyType>:$example_var
  );

  let optAssemblyFormat = [{
    `example` `(` $example_var `:` type($example_var) `)`
  }];

  let description = [{
    The `example_var` argument defines the variable to which the EXAMPLE clause
    applies.
  }];
}

def OpenMP_ExampleClause : OpenMP_ExampleClauseSkip<>;
```

### 添加操作

位于 [OpenMPOps.td](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/OpenMP/OpenMPOps.td) 中的 OpenMP 方言操作可以像任何其他常规操作一样定义，只需在继承 `OpenMP_Op` 时指定 `mnemonic` 和可选的 `traits` 列表，然后在其主体内定义预期的 `description`、`arguments` 等属性。然而，在大多数情况下，基于其接受的子句列表来定义操作会显著更简单，因为某些属性可以直接从这些子句继承。

一般来说，实现这一点的方式是：除了 `mnemonic` 和可选的 `traits` 列表外，还指定一个 `clauses` 列表，其中添加所有适用的 `OpenMP_<Name>Clause` 定义。然后，唯一需要在操作主体中定义的属性是 `summary` 和 `description`。对于后者，只需要定义操作本身，其子句继承参数的描述通过继承的 `clausesDescription` 属性附加。按照约定，操作的子句列表必须按字母顺序指定。

如果操作旨在有单个区域，最好通过设置 `OpenMP_Op` 的 `singleRegion=true` 模板参数来实现，而不是手动填充操作的 `regions` 属性，因为这样默认的 `assemblyFormat` 也会相应更新。

例如：

```tablegen
def ExampleOp : OpenMP_Op<"example", traits = [
    AttrSizedOperandSegments, ...
  ], clauses = [
    OpenMP_AlignedClause, OpenMP_IfClause, OpenMP_LinearClause, ...
  ], singleRegion = true> {
  let summary = "example construct";
  let description = [{
    The example construct represents...
  }] # clausesDescription;
}
```

这是可能的，因为操作的 `arguments`、`assemblyFormat` 和 `extraClassDeclaration` 属性默认通过连接列表上子句的相应属性来填充。对于 `assemblyFormat`，这涉及组合 `reqAssemblyFormat` 和 `optAssemblyFormat` 属性。所有子句的 `reqAssemblyFormat` 首先连接并用空格分隔，而 `optAssemblyFormat` 被包裹在 `oilist()` 中并用"|"而不是空格交织。生成的 `assemblyFormat` 包含必填汇编格式字符串，后跟可选汇编格式字符串，可选地包含 `$region` 和 `attr-dict`。

### 覆盖子句继承的属性

虽然基于子句的操作定义可以大大减少工作量，但它也有一定的限制，因为可能存在只有部分操作定义可以以这种方式自动化的情况。为了对从每个子句继承的属性进行细粒度控制，有两个功能可用：

  - 属性抑制。通过使用 `OpenMP_<Name>ClauseSkip` tablegen 类，可以选择从子句复制到操作的属性列表。例如，`OpenMP_IfClauseSkip<assemblyFormat = true>` 将导致除 `assemblyFormat` 之外的 `OpenMP_IfClause` 定义的所有属性用于初始填充操作的属性。
  - 属性增强。有时需要向子句填充的操作属性中添加内容。有一些内部属性定义来保存此默认值，而不是在操作定义中覆盖属性并必须手动复制本来会自动填充的内容：`clausesArgs`、`clausesAssemblyFormat`、`clauses{Req,Opt}AssemblyFormat` 和 `clausesExtraClassDeclaration`。

在以下示例中，假设 `OpenMP_InReductionClause` 和 `OpenMP_ReductionClause` 都定义了 `getReductionVars` 额外类声明，我们跳过两个子句继承的冲突 `extraClassDeclaration`，并提供另一个实现，而无需重新定义从 `OpenMP_AllocateClause` 继承的其他声明：

```tablegen
def ExampleOp : OpenMP_Op<"example", traits = [
    AttrSizedOperandSegments, ...
  ], clauses = [
    OpenMP_AllocateClause,
    OpenMP_InReductionClauseSkip<extraClassDeclaration = true>,
    OpenMP_ReductionClauseSkip<extraClassDeclaration = true>
  ], singleRegion = true> {
  let summary = "example construct";
  let description = [{
    This operation represents...
  }] # clausesDescription;

  // Override the clause-populated extraClassDeclaration and add the default
  // back via appending clausesExtraClassDeclaration to it. This has the effect
  // of adding one declaration. Since this property is skipped for the
  // InReduction and Reduction clauses, clausesExtraClassDeclaration won't
  // incorporate the definition of this property for these clauses.
  let extraClassDeclaration = [{
    SmallVector<Value> getReductionVars() {
      // Concatenate inReductionVars and reductionVars and return the result...
    }
  }] # clausesExtraClassDeclaration;
}
```

这些功能适用于复杂的边缘情况，但应尽量避免使用它们，因为它们可能会给方言引入不一致性和复杂性。

### Tablegen 验证传递

由于以这种方式隐式填充 MLIR 操作的基本属性，以及覆盖它们的能力，忘记附加子句继承的值可能会导致难以调试的 tablegen 错误。

为此，创建了 `-verify-openmp-ops` tablegen 伪后端。它在为 [OpenMPOps.td](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/OpenMP/OpenMPOps.td) 文件触发任何其他 tablegen 后端之前运行，每当在相应操作中找不到为子句定义的属性时发出警告，除非如[上文](#overriding-clause-inherited-properties)所述明确跳过。这样，如果稍后在处理 OpenMP 方言操作时发生 tablegen 失败，该传递触发的早期消息可以指向可能的解决方案。

### 操作数结构

基于每个适用子句定义的值和属性集合来表示操作的一个后果是，操作参数列表往往很长。这使得 C++ 操作构建器难以使用，且容易错误地以错误的顺序传递参数，这有时可能引入难以检测的问题。

提供给此问题的解决方案是操作数结构。其主要思想是为每个子句定义一个结构，其中包含一组字段，包含初始化与该子句关联的每个参数所需的数据。子句操作数结构通过类继承聚合到操作操作数结构中。然后，为每个操作定义一个自定义构建器，接受相应的操作数结构作为参数。由于每个参数都是结构的命名成员，因此设置所需参数以创建新操作变得简单得多。

可用于 ODS 定义自定义操作构建器的特定操作数结构可以在 [OpenMPClauseOperands.h](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/OpenMP/OpenMPClauseOperands.h) 中定义。然而，对于基于子句的操作定义，通常不需要这样做。在构建 'omp' 方言时触发的 `-gen-openmp-clause-ops` tablegen 后端将以以下方式自动生成结构：

- 它将为每个 `OpenMP_Clause` 定义创建一个 `<Name>ClauseOps` 结构，每个参数对应一个字段。
- 每个字段的名称将与相应参数的 tablegen 名称匹配，但将 snake_case 替换为 camel case。
- 字段的类型将从相应 tablegen 参数的类型获得：
  - 值用 `mlir::Value` 表示，`Variadic` 除外，它使其成为 `llvm::SmallVector<mlir::Value>`。
  - `OptionalAttr` 由其 `baseAttr` 的翻译表示。
  - 基于 `TypedArrayAttrBase` 的属性类型通过将其 `elementAttr` 的翻译包裹在 `llvm::SmallVector` 中来表示。此情况的唯一例外是如果 `elementAttr` 是"标量"（即非数组类型）属性类型，在这种情况下，将使用更通用的 `mlir::Attribute` 代替其 `storageType`。
  - 对于基于 `ElementsAttrBase` 的属性类型，尽力尝试获取元素类型（`llvm::APInt`、`llvm::APFloat` 或 `DenseArrayAttrBase` 的 `returnType`）以包裹在 `llvm::SmallVector` 中。如果无法获得（对于 `ElementsAttrBase` 的非内置直接子类会发生这种情况），将发出警告并使用 `storageType`（即特定的 `mlir::Attribute` 子类）。
  - 其他属性类型将以其 `storageType` 表示。
- 它将为每个操作创建 `<Name>Operands` 结构，这是一个继承相应 `OpenMP_Op` 子句的所有操作数结构的空结构。

### 条目块参数定义子句

在其 MLIR 表示中，某些 OpenMP 子句在它们应用到的操作与该 MLIR 操作区域的条目块参数之间引入映射。例如，这允许引入在 MLIR 操作外部定义的同一底层变量的私有副本。目前，具有此属性的子句可以分为三大类：
  - 类映射子句：`host_eval`（编译器内部，不由 OpenMP 规范定义：[见更多](#host-evaluated-clauses-in-target-regions)）、`map`、`use_device_addr` 和 `use_device_ptr`。
  - 类归约子句：`in_reduction`、`reduction` 和 `task_reduction`。
  - 私有化子句：`private`。

所有三类条目块参数定义子句使用类似的自定义汇编格式表示，仅在附加到每种类型的不同信息方面有所不同。以下是每种类型的示例：

```mlir
omp.target map_entries(%x -> %x.m, %y -> %y.m : !llvm.ptr, !llvm.ptr) {
  // Use %x.m, %y.m in place of %x and %y...
}

omp.wsloop reduction(@add.i32 %x -> %x.r, byref @add.f32 %y -> %y.r : !llvm.ptr, !llvm.ptr) {
  // Use %x.r, %y.r in place of %x and %y...
}

omp.parallel private(@x.privatizer %x -> %x.p, @y.privatizer %y -> %y.p : !llvm.ptr, !llvm.ptr) {
  // Use %x.p, %y.p in place of %x and %y...
}
```

由于这些子句的自定义汇编格式与操作的第一个区域条目块参数名称一起解析和打印，对于这些操作，条目块参数（即 `^bb0(...):` 行）不得被明确定义。此外，在允许每个子句独立解析和打印的同时实现此功能是不可能的，因为它们需要与对应操作的第一个区域一起打印/解析。它们还必须有一个明确定义的顺序，在此顺序中为给定操作指定这些子句中的多个。

这些子句与区域一起的解析/打印提供了在 `->` 之后直接定义条目块参数的能力。强制这些子句之间的特定顺序使块参数顺序定义明确，这是用于轻松将每个子句与其定义的条目块参数匹配的属性。

基于条目块参数定义子句的操作区域的自定义打印器和解析器基于 `{parse,print}BlockArgRegion` 函数实现，这些函数负责每种子句的排序和格式化，最小化由此方法导致的代码重复。以下是接受 `private` 和 `reduction` 子句的操作的自定义汇编格式示例：

```tablegen
let assemblyFormat = clausesAssemblyFormat # [{
  custom<PrivateReductionRegion>($region, $private_vars, type($private_vars),
      $private_syms, $reduction_vars, type($reduction_vars), $reduction_byref,
      $reduction_syms) attr-dict
}];
```

引入了 `BlockArgOpenMPOpInterface` 来简化这类子句的添加和处理。直接或通过子句间接将其添加到操作中，会为所有生成条目块参数的子句添加可覆盖的 `get<ClauseName>Vars()` 和 `num<ClauseName>BlockArgs()` 公共函数。默认情况下，子句定义的块参数报告数量将对应于操作为该子句接受的操作数数量。这个操作数列表默认为空，并将被同一子句定义的 `Variadic<...> $<clause_name>_vars` 参数的 getter 自动覆盖。

除了添加到实际操作的这些方法外，`BlockArgOpenMPOpInterface` 本身还基于之前的方法和条目块参数按子句名称字母顺序排序的约定定义了一组方法。这些列在下面，它们代表访问子句定义块参数的主要方式：
  - `get<ClauseName>BlockArgsStart()`：返回条目块参数列表中给定子句的第一个元素应位于的索引。
  - `get<ClauseName>BlockArgs()`：返回给定子句定义的条目块参数列表。
  - `numClauseBlockArgs()`：返回所有子句定义的条目块参数总数。
  - `getBlockArgsPairs()`：返回对列表，其中第一个元素是外部值或操作数，第二个元素是对应的条目块参数。

## 循环关联指令

循环关联的 OpenMP 构造在方言中表示为循环包装器操作。这些操作实现 `LoopWrapperInterface`，该接口对操作施加了一系列限制：
  - 它具有 `NoTerminator` 和 `SingleBlock` trait；
  - 它包含单个区域；以及
  - 其唯一块中恰好包含一个操作，该操作必须是另一个循环包装器或 `omp.loop_nest` 操作。

这种方法将循环巢的表示与指定如何执行其迭代的循环关联构造分开，可能跨越各种 SIMD 通道（`omp.simd`）、线程（`omp.wsloop`）、线程团队（`omp.distribute`）或任务（`omp.taskloop.wrapper`）。直接嵌套多个循环包装器以影响单个循环巢执行的能力用于以模块化方式表示复合构造。

`omp.loop_nest` 操作表示一个折叠的矩形循环巢，必须始终由至少一个循环包装器包裹，该包装器定义了它的执行方式。它作为 OpenMP 循环的更简单、更受限制的表示，同时正在开发基于新的 `omp.canonical_loop` 定义的更通用方法，以支持非矩形循环巢、循环转换和非完美嵌套循环。

以下示例展示了如何表示 `parallel {do,for}` 构造：
```mlir
omp.parallel ... {
  ...
  omp.wsloop ... {
    omp.loop_nest (%i) : index = (%lb) to (%ub) step (%step) {
      %a = load %a[%i] : memref<?xf32>
      %b = load %b[%i] : memref<?xf32>
      %sum = arith.addf %a, %b : f32
      store %sum, %c[%i] : memref<?xf32>
      omp.yield
    }
  }
  ...
  omp.terminator
}
```

### 循环转换

除了上述工作分配循环关联构造外，OpenMP 规范还定义了一组循环转换构造。它们在工作分配构造对生成的循环执行之前替换关联的循环。此类构造的一些示例是 `tile` 和 `unroll`。

表示这些类型的 OpenMP 构造的通用方法尚未实现，但与 `omp.canonical_loop` 工作密切相关。尽管如此，循环关联工作分配构造的 `collapse` 子句定义的循环转换可以通过向 `omp.loop_nest` 操作引入多个边界、步长和归纳变量来表示。

## 复合构造表示

OpenMP 规范定义了某些快捷方式，允许在单个指令中指定多个构造，这些被称为复合构造（例如，`parallel do` 包含 `parallel` 和 `do` 构造）。这些可以进一步分类为[组合构造](#combined-constructs)和[复合构造](#composite-constructs)。本节描述它们在方言中的表示方式。

当为复合构造指定子句时，OpenMP 规范定义了一组规则来决定它们应用于哪些叶构造，以及可能引入一些其他隐式子句。创建 MLIR 表示的人必须考虑这些规则，因为它是一种期望这些规则已经遵循的按叶表示。

### 组合构造

组合构造在语义上等价于在另一个构造内立即嵌套一个构造。利用此属性，通过与每个叶构造关联的操作来简化方言的表示。例如，`target teams` 将如下表示：

```mlir
omp.target ... {
  ...
  omp.teams ... {
    ...
    omp.terminator
  }
  ...
  omp.terminator
}
```

### 复合构造

复合构造类似于组合构造，因为它们指定了一个构造在另一个构造之后立即应用的效果。然而，它们将无法直接相互嵌套的构造组合在一起。具体来说，它们将适用于同一折叠循环巢的多个循环关联构造组合在一起。

截至 OpenMP 规范 5.2 版本，复合构造列表如下：
  - `{do,for} simd`；
  - `distribute simd`；
  - `distribute parallel {do,for}`；
  - `distribute parallel {do,for} simd`；以及
  - `taskloop simd`。

尽管复合构造列表相对较短，也可以为每个构造创建方言操作，但决定允许将多个循环包装器附加到单个循环。这最小化了方言中的冗余并最大化了其模块性，因为无论叶构造是否可以是复合构造的一部分，每个叶构造都有一个操作。另一方面，这意味着 `omp.loop_nest` 操作将不得不根据附加到它的循环包装器的数量和类型进行不同的解释。

为了简化检测参与复合构造表示的操作，引入了 `ComposableOpInterface`。其目的是处理可以选择性地附加到这些操作的 `omp.composite` 可丢弃方言属性。操作验证器将确保其存在与操作出现的上下文一致，因此当且仅当属性存在时表示复合构造的叶时是有效的。

例如，`distribute simd` 复合构造表示如下：

```mlir
omp.distribute ... {
  omp.simd ... {
    omp.loop_nest (%i) : index = (%lb) to (%ub) step (%step) {
      ...
      omp.yield
    }
  } {omp.composite}
} {omp.composite}
```

一个例外是 `distribute parallel {do,for}` 复合构造的表示。如果允许块关联的 `parallel` 叶构造作为循环包装器工作，则会引入许多问题。在这种情况下，改为使用"提升的 `omp.parallel` 表示"。这包括使 `omp.parallel` 成为父操作，嵌套的 `omp.loop_nest` 由 `omp.distribute` 和 `omp.wsloop`（在 `distribute parallel {do,for} simd` 情况下还有 `omp.simd`）包裹。

这种方法之所以有效，是因为 `parallel` 是并行性生成构造，而 `distribute` 是影响更高层次 `teams` 构造的工作分配构造，使这些构造之间的顺序不会导致语义不匹配。LLVM 的 SPMD 模式也利用了这个属性。

```mlir
omp.parallel ... {
  ...
  omp.distribute ... {
    omp.wsloop ... {
      omp.loop_nest (%i) : index = (%lb) to (%ub) step (%step) {
        ...
        omp.yield
      }
    } {omp.composite}
  } {omp.composite}
  ...
  omp.terminator
} {omp.composite}
```

## Target 区域中的主机求值子句

`omp.target` 操作（表示 OpenMP `target` 构造）被标记为 `IsolatedFromAbove` trait。这意味着，在其区域内，不能使用操作本身外部定义的 MLIR 值。这与 `target` 构造的 OpenMP 规范一致，该规范要求在 `target` 区域内使用的所有主机设备值必须是私有化的（数据共享）或映射的（数据映射）。

通常，应用于构造的子句在进入该构造之前被求值。此外，在某些情况下，OpenMP 规范规定子句应在进入父 `target` 构造时_在主机设备上_求值。特别是，如果 `teams` 构造嵌套在 `target` 构造内部或与之组合，则 `teams` 构造的 `num_teams` 和 `thread_limit` 子句必须在主机设备上求值。

此外，OpenMP 方言到 LLVM IR 翻译目标的运行时库支持 SPMD 内核的优化启动（即 OpenMP 中的 `target teams distribute parallel {do,for}`），这需要提前指定循环的总行程计数。因此，在内核启动之前在主机设备上求值行程计数也是有益的。

这些主机求值值在 MLIR 中需要放置在 `omp.target` 区域之外，并附加到对应的嵌套操作，由于 `IsolatedFromAbove` trait，这是不可能的。为解决这个问题，实现的解决方案是向 `omp.target` 操作引入 `host_eval` 参数。它的工作方式类似于 `map` 子句，但其唯一预期用途是将主机求值的值转发给区域内对应的操作。在之前描述之外的任何使用都会导致验证器错误。

```mlir
// Initialize %0, %1, %2, %3...
omp.target host_eval(%0 -> %nt, %1 -> %lb, %2 -> %ub, %3 -> %step : i32, i32, i32, i32) {
  omp.teams num_teams(to %nt : i32) {
    omp.parallel {
      omp.distribute {
        omp.wsloop {
          omp.loop_nest (%iv) : i32 = (%lb) to (%ub) step (%step) {
            // ...
            omp.yield
          }
          omp.terminator
        } {omp.composite}
        omp.terminator
      } {omp.composite}
      omp.terminator
    } {omp.composite}
    omp.terminator
  }
  omp.terminator
}
```
