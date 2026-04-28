# 编写 LLVM Pass（新 Pass 管理器版本）

> 原文：<https://llvm.org/docs/WritingAnLLVMNewPMPass.html>
> 版本：LLVM 23.0.0git，最后更新：2026-04-22

---

## 目录

1. [简介 — 什么是 Pass？](#简介--什么是-pass)
2. [快速入门 — 编写 Hello World](#快速入门--编写-hello-world)
   - [配置构建系统](#配置构建系统)
   - [所需的基本代码](#所需的基本代码)
   - [使用 opt 运行 Pass](#使用-opt-运行-pass)
   - [测试 Pass](#测试-pass)
3. [常见问题](#常见问题)
   - [必需 Pass（Required Passes）](#必需-pass)
   - [将 Pass 注册为插件](#将-pass-注册为插件)

---

## 简介 — 什么是 Pass？

> **警告**
>
> 本文档介绍的是**新 Pass 管理器**。LLVM 在代码生成流水线中使用的是旧版 Pass 管理器。
> 更多细节请参阅：
> - [编写 LLVM Pass（旧版 PM）](https://llvm.org/docs/WritingAnLLVMPass.html)
> - [使用新 Pass 管理器](https://llvm.org/docs/NewPassManager.html)

LLVM Pass 框架是 LLVM 系统的重要组成部分，因为编译器中最有趣的逻辑大多存在于 LLVM Pass 中。Pass 负责执行构成编译器的各种变换和优化，构建这些变换所使用的分析结果，并且最重要的是，它是一种组织编译器代码的结构化技术。

与旧版 Pass 管理器通过继承来定义 Pass 接口不同，新 Pass 管理器下的 Pass 依赖于**基于概念的多态性（concept-based polymorphism）**，即不存在显式的接口（详见 `PassManager.h` 中的注释）。所有 LLVM Pass 都继承自 CRTP 混入类 `PassInfoMixin<PassT>`。Pass 必须实现一个 `run()` 方法，该方法返回 `PreservedAnalyses`，并接受某个 IR 单元及其对应的分析管理器。例如，一个函数级 Pass 的方法签名为：

```cpp
PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
```

下面我们将介绍如何构建一个 Pass，包括配置构建系统、创建 Pass、执行和测试。阅读现有的 Pass 实现始终是学习细节的好方法。

---

## 快速入门 — 编写 Hello World

本节介绍如何编写"Hello World" Pass。`HelloWorld` Pass 的功能非常简单：打印出程序中所有非外部函数的名称。它完全不修改程序，只是对其进行检查。

下面的代码已经存在于仓库中；你也可以参照 HelloWorld 的源文件，创建一个使用不同名称的 Pass。

---

### 配置构建系统

首先，按照 [LLVM 入门指南](https://llvm.org/docs/GettingStarted.html) 中的说明配置并构建 LLVM。

接下来，我们将复用一个已有的目录（创建新目录需要修改更多 CMake 文件）。在本示例中，我们使用 `llvm/lib/Transforms/Utils/HelloWorld.cpp`，该文件已经创建好了。如果你想创建自己的 Pass，可以在 `llvm/lib/Transforms/Utils/CMakeLists.txt` 中添加一个新的源文件（假设你希望将 Pass 放在 `Transforms/Utils` 目录下）。

构建系统配置完成后，就可以编写 Pass 的代码了。

---

### 所需的基本代码

构建系统配置好之后，我们只需编写 Pass 本身。

首先，在头文件中定义 Pass。创建文件 `llvm/include/llvm/Transforms/Utils/HelloWorld.h`，内容如下所示的样板代码：

```cpp
#ifndef LLVM_TRANSFORMS_HELLONEW_HELLOWORLD_H
#define LLVM_TRANSFORMS_HELLONEW_HELLOWORLD_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class HelloWorldPass : public PassInfoMixin<HelloWorldPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_HELLONEW_HELLOWORLD_H
```

这段代码定义了 Pass 的类，并声明了实际执行 Pass 逻辑的 `run()` 方法。继承 `PassInfoMixin<PassT>` 会自动设置一些样板代码，无需手动编写。

我们的类位于 `llvm` 命名空间中，以避免污染全局命名空间。

接下来，创建 `llvm/lib/Transforms/Utils/HelloWorld.cpp`，以如下 include 语句开头：

```cpp
#include "llvm/Transforms/Utils/HelloWorld.h"
```

……用于包含刚刚创建的头文件。

```cpp
using namespace llvm;
```

……这是必需的，因为头文件中的函数定义在 `llvm` 命名空间中。此声明只应出现在非头文件中。

接下来是 Pass 的 `run()` 方法定义：

```cpp
PreservedAnalyses HelloWorldPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  errs() << F.getName() << "\n";
  return PreservedAnalyses::all();
}
```

……它只是将函数名打印到标准错误输出。Pass 管理器会确保该 Pass 在模块中的每个函数上运行。返回值 `PreservedAnalyses::all()` 表示，由于我们没有修改任何函数，所有的分析结果（如支配树）在该 Pass 执行后仍然有效。

Pass 本身的代码到此结束。为了"注册"这个 Pass，还需要在两个地方添加内容。

在 `llvm/lib/Passes/PassRegistry.def` 文件的 `FUNCTION_PASS` 段中添加：

```cpp
FUNCTION_PASS("helloworld", HelloWorldPass())
```

……这会以 `"helloworld"` 为名称注册该 Pass。

`llvm/lib/Passes/PassRegistry.def` 会被 `llvm/lib/Passes/PassBuilder.cpp` 多次 `#include`，用于各种目的。由于它会构造我们的 Pass，因此还需要在 `llvm/lib/Passes/PassBuilder.cpp` 中添加相应的 `#include`：

```cpp
#include "llvm/Transforms/Utils/HelloWorld.h"
```

这样 Pass 的所有必要代码就都齐全了，下面来编译并运行它。

---

### 使用 opt 运行 Pass

有了全新的 Pass 之后，我们可以构建 `opt` 并用它将一些 LLVM IR 传入 Pass 进行处理。

```bash
$ ninja -C build/ opt
# 或使用你自己的构建系统和构建目录

$ cat /tmp/a.ll
define i32 @foo() {
  %a = add i32 2, 3
  ret i32 %a
}

define void @bar() {
  ret void
}

$ build/bin/opt -disable-output /tmp/a.ll -passes=helloworld
foo
bar
```

Pass 成功运行，并按预期打印出了函数名！

---

### 测试 Pass

对 Pass 进行测试非常重要，可以防止未来出现回归问题。我们在 `llvm/test/Transforms/Utils/helloworld.ll` 添加一个 lit 测试。有关测试的更多信息，请参阅 [LLVM 测试基础设施指南](https://llvm.org/docs/TestingGuide.html)。

```bash
$ cat llvm/test/Transforms/Utils/helloworld.ll
; RUN: opt -disable-output -passes=helloworld %s 2>&1 | FileCheck %s

; CHECK: {{^}}foo{{$}}
define i32 @foo() {
  %a = add i32 2, 3
  ret i32 %a
}

; CHECK-NEXT: {{^}}bar{{$}}
define void @bar() {
  ret void
}

$ ninja -C build check-llvm
# 运行新测试以及所有其他 LLVM lit 测试
```

---

## 常见问题

### 必需 Pass

定义了返回 `true` 的静态 `isRequired()` 方法的 Pass 称为**必需 Pass（required pass）**。例如：

```cpp
class HelloWorldPass : public PassInfoMixin<HelloWorldPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  static bool isRequired() { return true; }
};
```

必需 Pass 是不可被跳过的 Pass。`AlwaysInlinerPass` 是必需 Pass 的一个典型例子，它必须始终运行以保证 `alwaysinline` 语义。Pass 管理器本身也是必需 Pass，因为它们可能包含其他必需 Pass。

Pass 被跳过的一个例子是函数属性 `optnone`，它表示不应对该函数运行优化。但必需 Pass 仍然会在带有 `optnone` 属性的函数上运行。

更多实现细节请参阅 `PassInstrumentation::runBeforePass()`。

---

### 将 Pass 注册为插件

LLVM 提供了一种机制，可以在 `clang` 或 `opt` 等工具中注册 Pass 插件。Pass 插件可以将 Pass 添加到默认优化流水线，或通过 `opt` 等工具手动运行。更多信息请参阅 [使用新 Pass 管理器](https://llvm.org/docs/NewPassManager.html)。

在仓库根目录下与其他项目并列创建一个 CMake 项目，该项目必须包含如下最简 `CMakeLists.txt`：

```cmake
add_llvm_pass_plugin(MyPassName source.cpp)
```

更多 CMake 细节请参阅 `add_llvm_pass_plugin` 的定义。

该 Pass 必须为新 Pass 管理器提供至少两个入口点之一，分别用于静态注册和动态加载插件：

- `llvm::PassPluginLibraryInfo get##Name##PluginInfo();`
- `extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() LLVM_ATTRIBUTE_WEAK;`

Pass 插件默认以动态方式编译和链接。将 `LLVM_${NAME}_LINK_INTO_TOOLS` 设置为 `ON` 可将项目转换为静态链接扩展。

树内示例请参见 `llvm/examples/Bye/`。

**让 PassBuilder 感知静态链接的 Pass 插件：**

```cpp
// 声明插件扩展函数。
#define HANDLE_EXTENSION(Ext) llvm::PassPluginLibraryInfo get##Ext##PluginInfo();
#include "llvm/Support/Extension.def"

...

// 在 PassBuilder 中注册插件扩展。
#define HANDLE_EXTENSION(Ext) get##Ext##PluginInfo().RegisterPassBuilderCallbacks(PB);
#include "llvm/Support/Extension.def"
```

**让 PassBuilder 感知动态链接的 Pass 插件：**

```cpp
// 动态加载插件。
auto Plugin = PassPlugin::Load(PathToPlugin);
if (!Plugin)
  report_error();
// 在 PassBuilder 中注册插件扩展。
Plugin.registerPassBuilderCallbacks(PB);
```

---

*© Copyright 2003-2026, LLVM Project. 原文最后更新于 2026-04-22。*
