# 'affine' 方言（Dialect）

本方言（dialect）为仿射操作（affine operations）和分析提供了强大的抽象。

[TOC]

## 多面体结构（Polyhedral Structures）

MLIR 采用多面体编译（polyhedral compilation）技术，使依赖分析和循环变换高效且可靠。本节介绍贯穿全文的若干核心概念。

### 维度（Dimensions）与符号（Symbols）

维度和符号是多面体结构中可以出现的两种标识符，其类型始终为 [`index`](Builtin.md/#indextype)。维度在圆括号中声明，符号在方括号中声明。

示例：

```mlir
// A 2d to 3d affine mapping.
// d0/d1 are dimensions, s0 is a symbol
#affine_map2to3 = affine_map<(d0, d1)[s0] -> (d0, d1 + s0, d1 - s0)>
```

维度标识符对应所表示的底层结构的维度（映射、集合，或更具体地，循环嵌套或张量）；例如，三维循环嵌套拥有三个维度标识符。符号标识符表示一个未知量，在感兴趣的区域内可视为常量。

在 MLIR 中，维度和符号通过各种操作（op）与 SSA 值绑定，并使用同样的圆括号与方括号列表来区分两者。

语法：

```
// Uses of SSA values that are passed to dimensional identifiers.
dim-use-list ::= `(` ssa-use-list? `)`

// Uses of SSA values that are used to bind symbols.
symbol-use-list ::= `[` ssa-use-list? `]`

// Most things that bind SSA values bind dimensions and symbols.
dim-and-symbol-use-list ::= dim-use-list symbol-use-list?
```

绑定到维度和符号的 SSA 值必须始终具有 'index' 类型。

示例：

```mlir
#affine_map2to3 = affine_map<(d0, d1)[s0] -> (d0, d1 + s0, d1 - s0)>
// Binds %N to the s0 symbol in affine_map2to3.
%x = memref.alloc()[%N] : memref<40x50xf32, #affine_map2to3>
```

### 维度与符号的限制

仿射方言对维度标识符和符号标识符施加了若干限制，以支持强大的分析和变换。如果某个 SSA 值满足以下任一条件，则其使用可以绑定到符号标识符：

1. 是具有 `AffineScope` 特性（trait）的操作（如 `FuncOp`）的区域（region）参数，
2. 是在 `AffineScope` 操作顶层定义的值（即直接被后者包围），
3. 是支配（dominate）包含该值使用处的 `AffineScope` 操作的值，
4. 是常量操作的结果，
5. 是操作数为合法符号标识符的 `Pure` 操作的结果，
6. 是对以下 memref 执行 [`dim` 操作](MemRef.md/#memrefdim-mlirmemrefdimop) 的结果：该 memref 是 `AffineScope` 操作的参数，或者对应维度是静态维度，或者是一个动态维度且已绑定到合法符号。

*注意：* 如果 SSA 值的使用不包含在任何具有 `AffineScope` 特性的操作中，则只能应用规则 4-6。

注意，根据规则 (3)，符号的合法性取决于 SSA 使用的位置。维度不仅可以绑定到符号可以绑定的所有对象，还可以绑定到外层 [`affine.for`](#affinefor-affineaffineforop) 和 [`affine.parallel`](#affineparallel-affineaffineparallelop) 操作的归纳变量，以及 [`affine.apply` 操作](#affineapply-affineaffineapplyop)的结果（该操作可递归地使用其他维度和符号）。

### 仿射表达式（Affine Expressions）

语法：

```
affine-expr ::= `(` affine-expr `)`
              | affine-expr `+` affine-expr
              | affine-expr `-` affine-expr
              | `-`? integer-literal `*` affine-expr
              | affine-expr `ceildiv` integer-literal
              | affine-expr `floordiv` integer-literal
              | affine-expr `mod` integer-literal
              | `-`affine-expr
              | bare-id
              | `-`? integer-literal

multi-dim-affine-expr ::= `(` `)`
                        | `(` affine-expr (`,` affine-expr)* `)`
```

`ceildiv` 是上取整除法函数，将第一个参数除以第二个参数的结果映射为大于或等于该结果的最小整数。`floordiv` 是下取整除法函数，将第一个参数除以第二个参数的结果映射为小于或等于该结果的最大整数。`mod` 是取模运算：由于第二个参数始终为正，本文中其结果也始终为正。`ceildiv`、`floordiv` 和 `mod` 的 `integer-literal` 操作数始终应为正数。`bare-id` 是一个标识符，其类型必须为 [index](Builtin.md/#indextype)。仿射表达式中各运算的优先级从高到低依次为：(1) 括号，(2) 取负，(3) 取模、乘法、floordiv 和 ceildiv，(4) 加法和减法。所有运算符均从左向右结合。

*多维仿射表达式* 是由逗号分隔的一维仿射表达式列表，整个列表用圆括号括起来。

**背景知识：** 非正式地，仿射函数是线性函数加常数。更正式地，定义在向量 $\vec{v} \in \mathbb{Z}^n$ 上的函数 $f$ 是 $\vec{v}$ 的多维仿射函数，若 $f(\vec{v})$ 可以表示为 $M \vec{v} + \vec{c}$ 的形式，其中 $M$ 是来自 $\mathbb{Z}^{m \times n}$ 的常数矩阵，$\vec{c}$ 是来自 $\mathbb{Z}$ 的常数向量。$m$ 是该仿射函数的维度。MLIR 进一步将仿射函数的定义扩展，允许对正整数常数进行 'floordiv'、'ceildiv' 和 'mod' 运算。多面体编译社区通常将这类仿射函数的扩展称为拟仿射函数（quasi-affine functions）。MLIR 使用术语"仿射映射"（affine map）来指代这些多维拟仿射函数。例如，$(i+j+1, j)$、$(i \mod 2, j+i)$、$(j, i/4, i \mod 4)$、$(2i+1, j)$ 是 $(i, j)$ 的二维仿射函数，而 $(i \cdot j, i^2)$、$(i \mod j, i/j)$ 则不是 $(i, j)$ 的仿射函数。

### 仿射映射（Affine Maps）

语法：

```
affine-map-inline
   ::= dim-and-symbol-value-lists `->` multi-dim-affine-expr
```

维度和符号列表中的标识符必须唯一。这些是唯一可以出现在 'multi-dim-affine-expr' 中的标识符。规格中含有一个或多个符号的仿射映射称为"符号仿射映射"，不含符号的则称为"非符号仿射映射"。

**背景知识：** 仿射映射是数学函数，将一组维度索引和符号的列表变换为结果列表，仿射表达式将索引和符号组合起来。仿射映射区分[索引和符号](#维度dimensions与符号symbols)，因为索引是调用仿射映射时（通过 [affine.apply](#affineapply-affineaffineapplyop) 等操作）的输入，而符号在建立映射时绑定（例如，在创建 memref 时建立内存[布局映射](Builtin.md/#layout)）。

仿射映射在 MLIR 的各种核心结构中使用。我们对其形式施加的限制支持强大的分析和变换，同时使表示对若干感兴趣的操作保持封闭性。

#### 具名仿射映射（Named affine mappings）

语法：

```
affine-map-id ::= `#` suffix-id

// Definitions of affine maps are at the top of the file.
affine-map-def    ::= affine-map-id `=` affine-map-inline
module-header-def ::= affine-map-def

// Uses of affine maps may use the inline form or the named form.
affine-map ::= affine-map-id | affine-map-inline
```

仿射映射可以在使用点内联定义，也可以提升到文件顶部并通过仿射映射定义赋予名称，然后按名称使用。

示例：

```mlir
// Affine map out-of-line definition and usage example.
#affine_map42 = affine_map<(d0, d1)[s0] -> (d0, d0 + d1 + s0 floordiv 2)>

// Use an affine mapping definition in an alloc operation, binding the
// SSA value %N to the symbol s0.
%a = memref.alloc()[%N] : memref<4x4xf32, #affine_map42>

// Same thing with an inline affine mapping definition.
%b = memref.alloc()[%N] : memref<4x4xf32, affine_map<(d0, d1)[s0] -> (d0, d0 + d1 + s0 floordiv 2)>>
```

### 半仿射映射（Semi-affine maps）

半仿射映射是仿射映射的扩展，允许对符号标识符进行乘法、`floordiv`、`ceildiv` 和 `mod` 运算。因此，半仿射映射是仿射映射的严格超集。

半仿射表达式的语法：

```
semi-affine-expr ::= `(` semi-affine-expr `)`
                   | semi-affine-expr `+` semi-affine-expr
                   | semi-affine-expr `-` semi-affine-expr
                   | symbol-or-const `*` semi-affine-expr
                   | semi-affine-expr `ceildiv` symbol-or-const
                   | semi-affine-expr `floordiv` symbol-or-const
                   | semi-affine-expr `mod` symbol-or-const
                   | bare-id
                   | `-`? integer-literal

symbol-or-const ::= `-`? integer-literal | symbol-id

multi-dim-semi-affine-expr ::= `(` semi-affine-expr (`,` semi-affine-expr)* `)`
```

上述语法中运算的优先级和结合性与[仿射表达式](#仿射表达式affine-expressions)相同。

半仿射映射的语法：

```
semi-affine-map-inline
   ::= dim-and-symbol-value-lists `->` multi-dim-semi-affine-expr
```

半仿射映射可以在使用点内联定义，也可以提升到文件顶部并通过半仿射映射定义赋予名称，然后按名称使用。

```
semi-affine-map-id ::= `#` suffix-id

// Definitions of semi-affine maps are at the top of file.
semi-affine-map-def ::= semi-affine-map-id `=` semi-affine-map-inline
module-header-def ::= semi-affine-map-def

// Uses of semi-affine maps may use the inline form or the named form.
semi-affine-map ::= semi-affine-map-id | semi-affine-map-inline
```

### 整数集（Integer Sets）

整数集是标识符列表上仿射约束的合取（conjunction）。与整数集关联的标识符分为两类：集合的维度标识符和集合的符号标识符。该集合被视为以其符号标识符为参数。在语法中，集合的维度标识符列表用圆括号括起来，符号用方括号括起来。

仿射约束的语法：

```
affine-constraint ::= affine-expr `>=` `affine-expr`
                    | affine-expr `<=` `affine-expr`
                    | affine-expr `==` `affine-expr`
affine-constraint-conjunction ::= affine-constraint (`,` affine-constraint)*
```

整数集可以在使用点内联定义，也可以提升到文件顶部并通过整数集定义赋予名称，然后按名称使用。

```
integer-set-id ::= `#` suffix-id

integer-set-inline
   ::= dim-and-symbol-value-lists `:` '(' affine-constraint-conjunction? ')'

// Declarations of integer sets are at the top of the file.
integer-set-decl ::= integer-set-id `=` integer-set-inline

// Uses of integer sets may use the inline form or the named form.
integer-set ::= integer-set-id | integer-set-inline
```

整数集的维度数是出现在集合维度列表中的标识符数。上述语法中出现的仿射约束非终结符只能包含来自 dims 和 symbols 的标识符。没有约束的集合是在集合所有维度上均无界的集合。

示例：

```mlir
// A example two-dimensional integer set with two symbols.
#set42 = affine_set<(d0, d1)[s0, s1]
   : (d0 >= 0, -d0 + s0 - 1 >= 0, d1 >= 0, -d1 + s1 - 1 >= 0)>

// Inside a Region
affine.if #set42(%i, %j)[%M, %N] {
  ...
}
```

`d0` 和 `d1` 对应集合的维度标识符，而 `s0` 和 `s1` 是符号标识符。

## 操作（Operations）

[include "Dialects/AffineOps.md"]

### `affine.dma_start` (mlir::AffineDmaStartOp)

语法：

```
operation ::= `affine.dma_start` ssa-use `[` multi-dim-affine-map-of-ssa-ids `]`, `[` multi-dim-affine-map-of-ssa-ids `]`, `[` multi-dim-affine-map-of-ssa-ids `]`, ssa-use `:` memref-type
```

`affine.dma_start` 操作启动一个非阻塞 DMA 操作，将数据从源 memref 传输到目标 memref。源 memref 和目标 memref 的维度不必相同，但必须具有相同的元素类型。操作数包括：源 memref 及其索引、目标 memref 及其索引、以元素数量（memref 元素类型的元素个数）表示的数据传输大小、带索引的标签（tag）memref，以及可选的末尾步长（stride）和每步元素数（number_of_elements_per_stride）参数。标签位置由 AffineDmaWaitOp 用于检查完成状态。源 memref、目标 memref 和标签 memref 的索引与任何 affine.load/store 的限制相同。特别地，每个 memref 维度的索引必须是循环归纳变量和符号的仿射表达式。可选的步长参数类型应为 'index'，指定较慢内存空间（内存空间 id 较低的内存空间）的步长，每隔 stride 传输 number_of_elements_per_stride 个元素，直到传输 %num_elements 个元素。步长参数要么都指定，要么都不指定。'num_elements' 的值必须是 'number_of_elements_per_stride' 的倍数。

示例 1：

例如，将内存空间 0 中 memref `%src` 在索引 `[%i + 3, %j]` 处的 256 个元素传输到内存空间 1 中 memref `%dst` 在索引 `[%k + 7, %l]` 处的 `DmaStartOp` 操作，可如下指定：

```mlir
%num_elements = arith.constant 256
%idx = arith.constant 0 : index
%tag = memref.alloc() : memref<1xi32, 4>
affine.dma_start %src[%i + 3, %j], %dst[%k + 7, %l], %tag[%idx],
  %num_elements :
    memref<40x128xf32, 0>, memref<2x1024xf32, 1>, memref<1xi32, 2>
```

示例 2：

如果指定了 `%stride` 和 `%num_elt_per_stride`，则 DMA 预期每隔 `%stride` 个元素从内存空间 0 传输 `%num_elt_per_stride` 个元素，直到传输 `%num_elements` 个元素为止。

```mlir
affine.dma_start %src[%i, %j], %dst[%k, %l], %tag[%idx], %num_elements,
  %stride, %num_elt_per_stride : ...
```

### `affine.dma_wait` (mlir::AffineDmaWaitOp)

语法：

```
operation ::= `affine.dma_wait` ssa-use `[` multi-dim-affine-map-of-ssa-ids `]`, ssa-use `:` memref-type
```

`affine.dma_wait` 操作阻塞，直到与标签元素 `%tag[%index]` 关联的 DMA 操作完成。`%tag` 是一个 memref，`%index` 必须是一个索引，且与任何 load/store 索引具有相同的限制。特别地，每个 memref 维度的索引必须是循环归纳变量和符号的仿射表达式。`%num_elements` 是与 DMA 操作关联的元素数量。

示例：

```mlir
affine.dma_start %src[%i, %j], %dst[%k, %l], %tag[%index], %num_elements :
  memref<2048xf32, 0>, memref<256xf32, 1>, memref<1xi32, 2>
...
...
affine.dma_wait %tag[%index], %num_elements : memref<1xi32, 2>
```
