# 'irdl' 方言（Dialect）

[TOC]

## 基础（Basics）

IRDL（*中间表示定义语言，Intermediate Representation Definition Language*）方言允许将 MLIR 方言定义为 MLIR 程序。嵌套操作用于表示方言结构：方言包含操作（Operation）、类型（Type）和属性（Attribute），这些元素又进一步包含类型参数、操作数（Operand）、结果（Result）等。每个概念都映射到 IRDL 方言中的 MLIR 操作，如以下示例方言所示：

```mlir
irdl.dialect @cmath {
    irdl.type @complex {
        %0 = irdl.is f32
        %1 = irdl.is f64
        %2 = irdl.any_of(%0, %1)
        irdl.parameters(%2)
    }

    irdl.operation @mul {
        %0 = irdl.is f32
        %1 = irdl.is f64
        %2 = irdl.any_of(%0, %1)
        %3 = irdl.parametric @cmath::@complex<%2>
        irdl.operands(%3, %3)
        irdl.results(%3)
    }
}
```

该程序定义了一个 `cmath` 方言，其中包含一个 `complex` 类型和一个 `mul` 操作。两者都使用静态单赋值（SSA）约束操作来表达对参数的约束。非正式地说，可以将这些 SSA 值视为约束变量，在约束求值时归约为单一类型。例如，`mul` 操作中存储在 `%2` 中的 `irdl.any_of` 结果，在该 `mul` 约束求值的整个过程中会坍缩为 `f32` 或 `f64` 之一。因此，`mul` 的两个操作数和结果必须具有相同的类型（而不仅仅是满足相同的约束）。更多信息，请参阅[约束与组合器](#约束与组合器)。

为了简化方言，IRDL 变量是 `mlir::Attribute` 的句柄（Handle）。为了支持对 `mlir::Type` 的操作，IRDL 将所有类型包装在 `mlir::TypeAttr` 属性中。

## 原则（Principles）

IRDL 的核心原则如下（无特定顺序）：

- **可移植性（Portability）。** IRDL 方言应是自包含的，使得方言可以轻松分发，同时对所使用的编译器基础设施（或 MLIR 的具体提交版本）的假设最小化。
- **内省性（Introspection）。** IRDL 方言定义机制应尽可能提供强大的内省能力。方言应尽可能易于操作、生成和分析。
- **运行时声明支持（Runtime declaration support）。** IRDL 方言的规范应支持通过动态注册或即时（JIT）编译在运行时加载。与动态工作流的兼容性不应妨碍将 IRDL 方言编译为预先（AOT）声明的能力。
- **可靠性（Reliability）。** IRDL 中的概念应保持一致性和可预测性，并尽可能注重高层次的简洁性。因此，通过验证的 IRDL 定义应能开箱即用，而未通过验证的定义在任何情况下都应提供清晰易懂的错误信息。

尽管 IRDL 简化了中间表示（IR）的定义，但它本身仍然是一种 IR，因此不要求对用户来说书写起来十分舒适。

## 约束与组合器（Constraints and combinators）

属性、类型和操作验证器（Verifier）以约束变量的形式表达。约束变量被定义为约束操作（如 `irdl.is` 或约束组合器）的结果。

约束变量具有变量的行为：因此，对同一约束变量的多次匹配只有在匹配的类型或属性与之前匹配的相同时才能成功。以下示例：

```mlir
irdl.type @foo {
    %ty = irdl.any_type
    irdl.parameters(param1: %ty, param2: %ty)
}
```

只有具有两个相同参数的类型才能成功匹配（`foo<i32, i32>` 会匹配，而 `foo<i32, i64>` 会失败，即使 i32 和 i64 单独都满足 `irdl.any_type` 约束）。这种约束变量机制允许轻松表达对类型或属性相等性的要求。

为了声明更复杂的验证器，IRDL 提供了约束组合器操作，如 `irdl.any_of`、`irdl.all_of` 或 `irdl.parametric`。这些组合器可用于将约束变量组合成新的约束变量。与所有约束变量的使用一样，其约束变量操作数会强制要求所匹配的类型或属性相等，如前一段所述。

## 使用场景动机（Motivating use cases）

为了说明 IRDL 背后的设计理由，以下列举了 IRDL 预期使用场景的示例（无特定顺序）：

- **模糊测试生成（Fuzzer generation）。** 借助声明式的验证器定义，可以将 IRDL 方言编译为编译器模糊测试器，使其只生成能通过验证器的程序。
- **编译器基础设施间的可移植方言（Portable dialects between compiler infrastructures）。** 一些编译器基础设施独立于 MLIR，但在其他方面与 IR 兼容。可移植的 IRDL 方言允许在 MLIR 和其他编译器基础设施之间共享方言定义，无需维护多个可能不同步的定义。
- **方言简化（Dialect simplification）。** 由于 IRDL 定义可以方便地进行机械化修改，可以根据实际使用的操作来简化方言定义，从而产生更小的编译器。
- **SMT 分析（SMT analysis）。** 由于 IRDL 方言定义是声明式的，其定义可以被降级（Lower）到 SMT 等替代表示，从而允许分析考虑验证器的变换行为。

## 操作（Operations）

[include "Dialects/IRDLOps.md"]
