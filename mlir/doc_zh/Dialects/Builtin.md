# 内置方言（Builtin Dialect）

内置方言（builtin dialect）包含一组核心属性（Attribute）、操作（Operation）和类型（Type），这些组件在极其广泛的领域和抽象层次中具有普适性。本方言中的许多组件也是核心中间表示（IR，Intermediate Representation）实现的关键基础。因此，该方言在每个 `MLIRContext` 中被隐式加载，并可直接供所有 MLIR 用户使用。

鉴于本方言的广泛影响以及 MLIR 在设计上的可扩展性，任何潜在的新增内容都会受到严格审查。

[TOC]

## 属性（Attributes）

[include "Dialects/BuiltinAttributes.md"]

## 位置属性（Location Attributes）

内置属性值的一个子集对应于[源代码位置（source locations）](../Diagnostics.md/#source-locations)，可以附加到操作（Operation）上。

[include "Dialects/BuiltinLocationAttributes.md"]

## DistinctAttribute

DistinctAttribute 将一个属性与唯一标识符关联起来。因此，多个 DistinctAttribute 实例可以指向同一个属性。每次调用 `create` 函数都会分配一个新的 DistinctAttribute 实例。该属性实例的地址用作临时唯一标识符。与静态单赋值（SSA，Static Single Assignment）值的名称类似，最终的唯一标识符在美化打印（pretty printing）期间生成。这种延迟编号机制确保即使多个 DistinctAttribute 实例并行创建，打印出的标识符也是确定性的。

语法：

```
distinct-id ::= integer-literal
distinct-attribute ::= `distinct` `[` distinct-id `]<` attribute `>`
```

示例：

```mlir
#distinct = distinct[0]<42.0 : f32>
#distinct1 = distinct[1]<42.0 : f32>
#distinct2 = distinct[2]<array<i32: 10, 42>>
```

该机制旨在生成带有唯一标识符的属性，可用于标记共享公共属性的操作组。例如，一组具有别名关系的内存操作可以使用每个别名组对应一个 DistinctAttribute 实例来进行标记。

## 操作（Operations）

[include "Dialects/BuiltinOps.md"]

## 类型（Types）

[include "Dialects/BuiltinTypes.md"]

## 类型接口（Type Interfaces）

[include "Dialects/BuiltinTypeInterfaces.md"]
