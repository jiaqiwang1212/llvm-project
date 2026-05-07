# Traits（特质）

[TOC]

MLIR 允许真正开放的生态系统，因为任何方言都可以定义适合特定抽象层次的属性、操作和类型。`Traits`（特质）是一种机制，用于抽象在许多不同属性/操作/类型等之间共同的实现细节和属性。`Traits` 可用于指定对象的特殊属性和约束，包括某个操作是否有副作用，或其输出与输入具有相同类型。操作 trait 的一些示例有 `Commutative`（可交换）、`Terminator`（终止符）等。有关更多示例，请参阅下方更全面的[操作 traits 列表](#operation-traits-list)。

## 定义 Trait

Traits 可以在 C++ 中通过继承特定 IR 类型的 `TraitBase<ConcreteType, TraitType>` 类来定义。对于属性，这是 `AttributeTrait::TraitBase`；对于操作，这是 `OpTrait::TraitBase`；对于类型，这是 `TypeTrait::TraitBase`。该基类接受以下模板参数：

*   ConcreteType
    -   该 trait 所附加的具体类类型。
*   TraitType
    -   正在定义的 trait 类的类型，与[`奇异递归模板模式`](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)配合使用。

派生的 trait 类预期接受一个对应于 `ConcreteType` 的单一模板参数。下面是一个 trait 定义示例：

```c++
template <typename ConcreteType>
class MyTrait : public TraitBase<ConcreteType, MyTrait> {
};
```

操作 trait 还可以提供 `verifyTrait` 或 `verifyRegionTrait` 钩子，在验证具体操作时被调用。两者的区别在于验证器是否需要访问 region——如果需要，region 中的操作将在验证此 trait 之前先完成验证。[验证顺序](../DefiningDialects/Operations.md/#verification-ordering)决定了验证器被调用的时机。

```c++
template <typename ConcreteType>
class MyTrait : public OpTrait::TraitBase<ConcreteType, MyTrait> {
public:
  /// Override the 'verifyTrait' hook to add additional verification on the
  /// concrete operation.
  static LogicalResult verifyTrait(Operation *op) {
    // ...
  }
};
```

注意：通常最好将 `verifyTrait` 或 `verifyRegionTrait` 钩子的实现以独立函数的形式定义在类外，以避免为每种具体操作类型实例化该实现。

操作 trait 还可以提供 `foldTrait` 钩子，在折叠具体操作时被调用。只有当具体操作的折叠未实现、失败或执行原地折叠时，trait 折叠器才会被调用。

如果实现了以下签名的折叠，且操作有单个结果，则该折叠将被调用：

```c++
template <typename ConcreteType>
class MyTrait : public OpTrait::TraitBase<ConcreteType, MyTrait> {
public:
  /// Override the 'foldTrait' hook to support trait based folding on the
  /// concrete operation.
  static OpFoldResult foldTrait(Operation *op, ArrayRef<Attribute> operands) {
    // ...
  }
};
```

否则，如果操作有单个结果但上述签名未实现，或操作有多个结果，则将使用以下签名（如果已实现）：

```c++
template <typename ConcreteType>
class MyTrait : public OpTrait::TraitBase<ConcreteType, MyTrait> {
public:
  /// Override the 'foldTrait' hook to support trait based folding on the
  /// concrete operation.
  static LogicalResult foldTrait(Operation *op, ArrayRef<Attribute> operands,
                                 SmallVectorImpl<OpFoldResult> &results) {
    // ...
  }
};
```

注意：通常最好将 `foldTrait` 钩子的实现以独立函数的形式定义在类外，以避免为每种具体操作类型实例化该实现。

### 额外声明与定义

trait 可能需要在指定该 trait 的 Operation、Attribute 或 Type 实例上直接添加额外的声明和定义。`NativeTrait` 类中的 `extraConcreteClassDeclaration` 和 `extraConcreteClassDefinition` 字段是专为将代码直接注入生成的 C++ Operation、Attribute 或 Type 类而设计的机制。

`extraConcreteClassDeclaration` 字段中的代码将被格式化并复制到生成的 C++ Operation、Attribute 或 Type 类中。`extraConcreteClassDefinition` 中的代码将被添加到生成的源文件中，位于该类的 C++ 命名空间内。替换符 `$cppClass` 会被替换为 C++ 类名。

其目的是将特定于 trait 的逻辑集中在一起，减少实例上冗余的额外声明和定义。

### 参数化 Traits

以上示例演示了一个简单的自包含 trait 的定义。有时，向 trait 提供一些静态参数来控制其行为也非常有用。由于 trait 类的定义是固定的（即必须有一个用于具体对象的模板参数），参数的模板需要分离出来。示例如下：

```c++
template <int Parameter>
class MyParametricTrait {
public:
  template <typename ConcreteType>
  class Impl : public TraitBase<ConcreteType, Impl> {
    // Inside of 'Impl' we have full access to the template parameters
    // specified above.
  };
};
```

## 附加 Trait

在定义派生对象类型时可以使用 trait，只需将 trait 类的名称附加到基础对象类操作类型的末尾即可：

```c++
/// Here we define 'MyAttr' along with the 'MyTrait' and `MyParametric trait
/// classes we defined previously.
class MyAttr : public Attribute::AttrBase<MyAttr, ..., MyTrait, MyParametricTrait<10>::Impl> {};
/// Here we define 'MyOp' along with the 'MyTrait' and `MyParametric trait
/// classes we defined previously.
class MyOp : public Op<MyOp, MyTrait, MyParametricTrait<10>::Impl> {};
/// Here we define 'MyType' along with the 'MyTrait' and `MyParametric trait
/// classes we defined previously.
class MyType : public Type::TypeBase<MyType, ..., MyTrait, MyParametricTrait<10>::Impl> {};
```

### 在 ODS 中附加操作 Trait

要在 [ODS](../DefiningDialects/Operations.md) 框架中使用操作 trait，需要提供 trait 类的定义。这可以通过 `NativeOpTrait` 和 `ParamNativeOpTrait` 类来完成。`ParamNativeOpTrait` 提供了一种机制，用于为带有内部 `Impl` 的参数化 trait 类指定参数。

```tablegen
// The argument is the c++ trait class name.
def MyTrait : NativeOpTrait<"MyTrait">;

// The first argument is the parent c++ class name. The second argument is a
// string containing the parameter list.
class MyParametricTrait<int prop>
  : NativeOpTrait<"MyParametricTrait", !cast<string>(!head(parameters))>;
```

然后可以在操作定义的 `traits` 列表中使用它们：

```tablegen
def OpWithInferTypeInterfaceOp : Op<...[MyTrait, MyParametricTrait<10>]> { ... }
```

有关更多详情，请参阅[操作定义](../DefiningDialects/Operations.md)的文档。

## 使用 Trait

Traits 可用于直接在具体对象上提供额外的方法、静态字段或其他信息。`Traits` 在内部成为具体操作的 `Base` 类，因此所有这些信息都可以直接访问。若要以不透明的方式向转换和分析公开这些信息，可以使用 [`interfaces`（接口）](../Interfaces.md)。

要查询特定对象是否包含特定 trait，可以使用 `hasTrait<>` 方法。它接受 trait 类作为模板参数，与附加 trait 到操作时传入的类相同。

```c++
Operation *op = ..;
if (op->hasTrait<MyTrait>() || op->hasTrait<MyParametricTrait<10>::Impl>())
  ...;
```

## 操作 Traits 列表

MLIR 提供了一套 trait，这些 trait 提供了许多不同操作中常见的各种功能。以下是一些可被任何方言直接使用的关键 trait 列表。每个 trait 部分标题的格式如下：

*   `标题`
    -   （`C++ 类` -- `ODS 类`（如适用））

### AffineScope（仿射作用域）

*   `OpTrait::AffineScope` -- `AffineScope`

该 trait 由持有 region 的操作携带，用于为多面体优化及仿射方言定义一个新作用域。在这类操作之前占主导地位的 `index` 类型 SSA 值、在这类操作顶层定义的 SSA 值，或作为这类操作的 region 参数出现的 SSA 值，会自动成为该操作所定义多面体作用域的有效符号。因此，这些 SSA 值可以用作各种仿射方言操作（如 affine.for、affine.load 和 affine.store）的操作数或索引操作数。携带此 trait 的操作所定义的多面体作用域包含其 region 中的所有操作，但嵌套在其他同样携带此 trait 的操作内部的操作除外。

### AutomaticAllocationScope（自动分配作用域）

*   `OpTrait::AutomaticAllocationScope` -- `AutomaticAllocationScope`

该 trait 由持有 region 的操作携带，用于为自动分配定义一个新作用域。当控制权从这类操作的 region 返回时，此类分配会自动释放。例如，由 [`memref.alloca`](../Dialects/MemRef.md/#memrefalloca-memrefallocaop) 执行的分配，会在控制权离开其最近的携带 AutomaticAllocationScope trait 的外围操作的 region 时自动释放。

### Broadcastable（可广播）

*   `OpTrait::ResultsBroadcastableShape` -- `ResultsBroadcastableShape`

该 trait 添加了以下属性：操作的操作数已知具有[广播兼容性](https://docs.scipy.org/doc/numpy/user/basics.broadcasting.html)，且其结果类型与推断的广播形状兼容。详情请参阅 [`Broadcastable` Trait](Broadcastable.md)。

### Commutative（可交换）

*   `OpTrait::IsCommutative` -- `Commutative`

该 trait 添加了以下属性：操作是可交换的，即 `X op Y == Y op X`。

### ElementwiseMappable（逐元素可映射）

*   `OpTrait::ElementwiseMappable` -- `ElementwiseMappable`

该 trait 标记了那些也可以应用于向量/张量的标量操作，其在向量/张量上的语义为逐元素应用。该 trait 建立了一组属性，允许在标量/向量/张量代码之间进行推理和转换。这些相同的属性还允许为所有 `ElementwiseMappable` 操作提供各种分析/转换的统一实现。

注意：并非所有在某种抽象意义上"逐元素"的操作都满足此 trait。特别地，广播行为是不允许的。有关精确要求，请参阅 `OpTrait::ElementwiseMappable` 上的注释。

### HasParent（有父操作）

*   `OpTrait::HasParent<typename ParentOpType>` -- `HasParent<string op>` 或
    `ParentOneOf<list<string> opList>`

该 trait 为只能嵌套在附加到 `ParentOpType` 类型操作的 region 中的操作提供 API 和验证器。

### IsolatedFromAbove（与上层隔离）

*   `OpTrait::IsIsolatedFromAbove` -- `IsolatedFromAbove`

该 trait 表明操作的 region 已知与上层隔离。此 trait 断言操作的 region 不会捕获或引用在 region 作用域之上定义的 SSA 值。这意味着，如果 `foo.region_op` 被定义为 `IsolatedFromAbove`，则以下代码是无效的：

```mlir
%result = arith.constant 10 : i32
foo.region_op {
  foo.yield %result : i32
}
```

该 trait 是 IR 的重要结构属性，它使得操作可以在其下方调度 [pass](../PassManagement)。

### MemRefsNormalizable（MemRef 可规范化）

*   `OpTrait::MemRefsNormalizable` -- `MemRefsNormalizable`

该 trait 用于标记那些消费或产生 `MemRef` 类型值的操作，其中这些引用可以被"规范化"。如果关联的 `MemRef` 具有非恒等内存布局规范，此类可规范化操作可以被修改，使得 `MemRef` 具有恒等布局规范。这可以通过将该操作与其自身的索引表达式关联来实现，该表达式能够等效地表达 MemRef 类型的内存布局规范。请参阅 [the -normalize-memrefs pass](../Passes.md/#-normalize-memrefs)。

### Single Block Region（单块 Region）

*   `OpTrait::SingleBlock` -- `SingleBlock`

该 trait 为拥有单个块的 region 的操作提供 API 和验证器。

### Single Block with Implicit Terminator（带隐式终止符的单块 Region）

*   `OpTrait::SingleBlockImplicitTerminator<typename TerminatorOpType>` --
    `SingleBlockImplicitTerminator<string op>`

该 trait 隐含了上述 `SingleBlock`，但额外要求单个块必须以 `TerminatorOpType` 结束。

### SymbolTable（符号表）

*   `OpTrait::SymbolTable` -- `SymbolTable`

该 trait 用于定义 [`SymbolTable`](../SymbolsAndSymbolTables.md/#symbol-table) 的操作。

### Terminator（终止符）

*   `OpTrait::IsTerminator` -- `Terminator`

该 trait 为已知是[终止符](../LangRef.md/#control-flow-and-ssacfg-regions)的操作提供验证和功能。

*   `OpTrait::NoTerminator` -- `NoTerminator`

该 trait 取消了对操作所持有的 region 在块末尾必须有[终止符操作](../LangRef.md/#control-flow-and-ssacfg-regions)的要求。这要求这些 region 只有一个块。使用此 trait 的操作示例是顶层 `ModuleOp`。
