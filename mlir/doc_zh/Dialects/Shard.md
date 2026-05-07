# 'shard' 方言（Dialect）

'shard' 方言（Dialect）定义了一组属性（Attribute）、操作（Operation）和接口（Interface），用于处理张量分片（tensor sharding）和设备通信。

它受 [GSPMD](*通用可扩展的机器学习计算图并行化*) 的启发。

该方言最初名为 `mesh`，后来更名以更好地反映其实际功能。

[TOC]

## 集合通信操作（Collective Communication Operations）

'shard' 方言包含若干集合操作（collective operation），用于协调网格（grid）中各设备之间的通信。

如果你还不熟悉集合操作，可以参阅[这篇 Wikipedia 文章](https://en.wikipedia.org/wiki/Collective_operation)作为入门。

与传统的基于消息传递（message-passing）在每个进程的显式缓冲区之间定义的集合操作不同，本方言中的集合操作工作在更高层次。它们以数据在张量维度（dimension）上的移动方式来定义，参与的进程是从张量的分片方式中推断出来的，而非手动指定。

### 设备组（Device Groups）

集合操作在设备组内运行，这些组由 `grid` 和 `grid_axes` 属性来定义。这些属性描述了如何将完整的设备网格切分为更小的组。

在列出的 `grid_axes` **之外**坐标相同的设备属于同一组。

示例：假设你的设备网格形状为 `2×3×4×5`，并设置 `grid_axes = [0, 1]`。这将通过固定轴 2 和 3 将网格分成若干组，得到如下结构的组：

```
{ { (i, j, k, m) | 0 ≤ i < 2, 0 ≤ j < 3 } | 0 ≤ k < 4, 0 ≤ m < 5 }
```

因此，这些组由坐标 `(k, m)` 标识，设备 `(1, 0, 2, 3)` 和 `(1, 1, 2, 3)` 属于同一组，但 `(1, 0, 2, 4)` 属于不同的组。

对于某些集合操作（如 `all-to-all`），组内设备的顺序很重要。设备顺序基于 `grid_axes` 中轴的顺序，从最外层到最内层。

示例：若 `grid_axes = [3, 1]`，则设备 `(i, 1, k, 0)` 排在 `(i, 0, k, 1)` 和 `(i, 2, k, 0)` 之前。

### 组内设备（In-group Devices）

某些操作（如 `broadcast`、`scatter` 和 `send`）需要引用每个组中的特定设备。这些组内设备通过其在 `grid_axes` 所列轴上的坐标来标识。

示例：在一个 3D 网格中，若 `grid_axes = [0, 2]`，则组内设备用 `(i, j)` 指定。若一个组在轴 1 上的坐标固定为 `g`，则完整的设备索引为 `(i, g, j)`。

### 纯粹性与执行模型（Purity and Execution Model）

涉及一个组内所有设备的集合操作（如 `all-gather`、`all-to-all`）被视为纯操作（pure）。而 `send` 和 `recv` 这类操作不是集合操作，也不是纯操作。

执行模型假设为 SPMD（单程序多数据，Single Program Multiple Data）：

* 每个进程运行相同的程序。
* 在任何集合操作处，所有进程保持同步。

这意味着编译器优化必须谨慎处理集合操作。例如，如果在优化过程中移除了一个集合操作，则必须从*每条*路径和*每个*将参与该操作的进程中移除——否则会在运行时产生未定义行为。

将这些操作标记为纯操作也有助于标准编译器 pass（遍）（如死代码消除和公共子表达式消除）的执行。它确保程序运行时，所有设备在集合操作处同时执行同一行代码，从而避免死锁。

## 操作（Operations）

[include "Dialects/ShardOps.md"]

## 属性（Attributes）

[include "Dialects/ShardAttrs.md"]
