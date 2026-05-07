# 符号（Symbol）与符号表（Symbol Table）

[TOC]

借助[区域（Region）](LangRef.md/#regions)，MLIR 的多层级特性在中间表示（IR）中以结构化方式体现。编译器中的大量基础设施围绕这种嵌套结构构建，包括[通道管理器（pass manager）](PassManagement.md/#pass-manager)中对操作（op）的处理。MLIR 设计的一个优势在于能够利用多线程并行处理操作。这得益于 IR 的一个属性——[`IsolatedFromAbove`](Traits/#isolatedfromabove)。

若不具备该属性，任何操作都可能影响或修改定义在其上方的操作的使用列表（use-list）。要使其线程安全，需要在某些核心 IR 数据结构上加入开销较大的锁，效率较低。为了在不使用锁的情况下实现多线程编译，MLIR 对常量值使用本地池，对全局值和变量使用 `Symbol` 访问。本文档详细介绍 `Symbol` 的设计、其含义以及与系统的集成方式。

`Symbol` 基础设施本质上提供了一种非静态单赋值（non-SSA）机制，用于通过名称以符号方式引用操作。这允许安全地引用定义在 `IsolatedFromAbove` 区域之上的操作，同时也允许符号化引用定义在其他区域之下的操作。

## 符号（Symbol）

`Symbol` 是一个具名操作，直接驻留在定义了 [`SymbolTable`](#symbol-table) 的区域内。符号的名称在父级 `SymbolTable` 中必须唯一。该名称在语义上类似于 SSA 结果值，可被其他操作引用，以提供对该符号的符号链接或使用。`Symbol` 操作的一个例子是 [`func.func`](Dialects/Builtin.md/#func-mlirfuncop)。`func.func` 定义一个符号名称，该名称由 [`func.call`](Dialects/Func.md/#funccall-callop) 等操作[引用](#referencing-a-symbol)。

### 定义或声明符号

`Symbol` 操作应使用 `SymbolOpInterface` 接口来提供必要的验证和访问器；它还支持 `builtin.module` 等条件性定义符号的操作。`Symbol` 必须具有以下属性：

*   一个名为 `'SymbolTable::getSymbolAttrName()'`（`sym_name`）的 `StringAttr` 属性。
    -   该属性定义操作的符号"名称"。
*   一个可选的名为 `'SymbolTable::getVisibilityAttrName()'`（`sym_visibility`）的 `StringAttr` 属性。
    -   该属性定义符号的[可见性](#symbol-visibility)，更具体地说，是指它可以在哪些作用域中被访问。
*   无 SSA 结果
    -   混合使用不同的 `use` 操作方式会很快变得难以管理和分析。
*   该操作是声明还是定义（`isDeclaration`）
    -   声明不定义新符号，而是引用在可见 IR 之外定义的符号。

## 符号表（Symbol Table）

上文描述的 `Symbol` 驻留在定义了 `SymbolTable` 的操作的区域内。`SymbolTable` 操作为 [`Symbol`](#symbol) 操作提供容器。它验证所有 `Symbol` 操作具有唯一名称，并提供按名称查找符号的功能。定义 `SymbolTable` 的操作必须使用 `OpTrait::SymbolTable` 特征（trait）。

### 引用符号

`Symbol` 通过 [`SymbolRefAttr`](Dialects/Builtin.md/#symbolrefattr) 属性以名称方式进行符号化引用。符号引用属性包含对嵌套在符号表中的操作的命名引用，还可以选择性地包含一组嵌套引用，进一步解析到不同符号表中嵌套的符号。在解析嵌套引用时，每个非叶引用必须指向同样是[符号表](#symbol-table)的符号操作。

下面是一个操作如何引用符号操作的示例：

```mlir
// This `func.func` operation defines a symbol named `symbol`.
func.func @symbol()

// Our `foo.user` operation contains a SymbolRefAttr with the name of the
// `symbol` func.
"foo.user"() {uses = [@symbol]} : () -> ()

// Symbol references resolve to the nearest parent operation that defines a
// symbol table, so we can have references with arbitrary nesting levels.
func.func @other_symbol() {
  affine.for %i0 = 0 to 10 {
    // Our `foo.user` operation resolves to the same `symbol` func as defined
    // above.
    "foo.user"() {uses = [@symbol]} : () -> ()
  }
  return
}

// Here we define a nested symbol table. References within this operation will
// not resolve to any symbols defined above.
module {
  // Error. We resolve references with respect to the closest parent operation
  // that defines a symbol table, so this reference can't be resolved.
  "foo.user"() {uses = [@symbol]} : () -> ()
}

// Here we define another nested symbol table, except this time it also defines
// a symbol.
module @module_symbol {
  // This `func.func` operation defines a symbol named `nested_symbol`.
  func.func @nested_symbol()
}

// Our `foo.user` operation may refer to the nested symbol, by resolving through
// the parent.
"foo.user"() {uses = [@module_symbol::@nested_symbol]} : () -> ()
```

使用属性（attribute）而非 SSA 值有以下几个优点：

*   引用可以出现在比操作数列表更多的地方，包括[嵌套属性字典](Dialects/Builtin.md/dictionaryattr)、[数组属性](Dialects/Builtin.md/#arrayattr)等。

*   SSA 支配（dominance）的处理保持不变。

    -   如果使用 SSA 值，需要创建某种机制来选择退出其某些属性（如支配性）。属性允许无论操作的定义顺序如何都能引用操作。
    -   属性简化了对嵌套符号表中操作的引用，而这些操作在传统上在父区域之外不可见。

这种选择使用属性而非 SSA 值的方案带来的影响是：我们现在有两种机制来引用操作。这意味着某些方言必须同时支持 `SymbolRefs` 和 SSA 值引用，或者提供从符号引用物化（materialize）SSA 值的操作。每种方式根据具体情况各有权衡。函数调用可以直接使用 `SymbolRef` 作为被调用者，而对全局变量的引用可能使用物化操作，以便在 `arith.addi` 等其他操作中使用该变量。[`llvm.mlir.addressof`](Dialects/LLVM.md/#llvmmliraddressof-mlirllvmaddressofop) 就是这样一个操作的例子。

有关该属性结构的更多信息，请参阅 `LangRef` 中 [`SymbolRefAttr`](Dialects/Builtin.md/#symbolrefattr) 的定义。

引用 `Symbol` 并希望执行符号验证和一般性变更的操作应实现 `SymbolUserOpInterface`，以确保符号访问合法且高效。

### 操作符号

如上所述，`SymbolRefs` 是传统 SSA 使用列表之外的一种辅助操作定义使用关系的方式。因此，提供类似功能以操作和检查使用列表及使用者至关重要。以下是 `SymbolTable` 提供的部分实用工具：

*   `SymbolTable::getSymbolUses`

    -   访问特定操作上及其嵌套内所有使用的迭代器范围。

*   `SymbolTable::symbolKnownUseEmpty`

    -   检查在 IR 的特定部分中，某个符号是否已知未被使用。

*   `SymbolTable::replaceAllSymbolUses`

    -   在 IR 的特定部分中，将某个符号的所有使用替换为新符号。

*   `SymbolTable::lookupNearestSymbolFrom`

    -   从某个锚操作（anchor operation）向上查找最近符号表中的符号定义。

## 符号可见性

除名称外，`Symbol` 还附带一个 `visibility`（可见性）。符号的可见性定义了其在 IR 中的结构可达性。符号具有以下几种可见性之一：

*   公开（Public，默认）

    -   该符号可以从可见 IR 之外被引用。我们不能假设该符号的所有使用都是可观察的。如果操作是声明一个符号（而非定义），则不允许公开可见性，因为符号声明不打算从可见 IR 之外使用。

*   私有（Private）

    -   该符号只能从当前符号表内部引用。

*   嵌套（Nested）

    -   该符号可以被当前符号表之外的操作引用，但不能超出可见 IR 的范围，前提是每个父级符号表也定义了非私有符号。

对于函数而言，可见性在操作名称之后打印，不带引号。以下是 IR 中几个示例：

```mlir
module @public_module {
  // This function can be accessed by 'live.user', but cannot be referenced
  // externally; all uses are known to reside within parent regions.
  func.func nested @nested_function()

  // This function cannot be accessed outside of 'public_module'.
  func.func private @private_function()
}

// This function can only be accessed from within the top-level module.
func.func private @private_function()

// This function may be referenced externally.
func.func @public_function()

"live.user"() {uses = [
  @public_module::@nested_function,
  @private_function,
  @public_function
]} : () -> ()
```
