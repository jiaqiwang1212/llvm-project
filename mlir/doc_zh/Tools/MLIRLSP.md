# MLIR：语言服务器协议

[TOC]

本文档描述了支持各种 MLIR 相关语言的 [LSP](https://microsoft.github.io/language-server-protocol/) IDE 语言扩展的工具和实用程序。LSP 语言扩展通常由两个组件组成：语言客户端和语言服务器。语言客户端是与您使用的 IDE（如 VSCode）交互的一段代码。语言服务器充当客户端可能希望执行的查询的后端，例如"查找定义"、"查找引用"等。

## MLIR LSP 语言服务器：`mlir-lsp-server`

MLIR 以 `mlir-lsp-server` 工具的形式为 `.mlir` 文本文件提供 LSP 语言服务器的实现。该工具与 MLIR C++ API 交互，以支持丰富的语言查询，例如"查找定义"。

### 支持自定义方言

`mlir-lsp-server` 与许多其他基于 MLIR 的工具一样，依赖于注册适当的方言，以便能够解析文本 `.mlir` 文件中使用的自定义汇编格式。主 MLIR 仓库中的 `mlir-lsp-server` 为所有上游 MLIR 方言提供支持。下游和树外用户需要提供一个自定义的 `mlir-lsp-server` 可执行文件，注册他们感兴趣的实体。`mlir-lsp-server` 的实现作为库提供，使下游用户可以轻松注册其方言并直接调用主实现。以下是一个简单示例：

```c++
#include "mlir/Tools/mlir-lsp-server/MlirLspServerMain.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registerMyDialects(registry);
  return mlir::failed(mlir::MlirLspServerMain(argc, argv, registry));
}
```

有关如何在几个已知的 LSP 客户端（如 vscode）中设置支持的详细信息，请参阅下面的[编辑器插件](#editor-plugins)部分。

### 功能

本节详细介绍 MLIR 语言服务器提供的几个功能。截图显示在 [VSCode](https://code.visualstudio.com/) 中，但可用的确切功能集取决于您的编辑器客户端。

[mlir features]: #

#### 诊断

语言服务器在您键入时主动运行 IR 验证，并就地显示任何生成的诊断。

![IMG](/mlir-lsp-server/diagnostics.png)

##### 自动插入 `expected-` 诊断检查

MLIR 提供了用于检查预期诊断的[基础设施](https://mlir.llvm.org/docs/Diagnostics/#sourcemgr-diagnostic-verifier-handler)，在定义 IR 解析和验证时被广泛使用。语言服务器提供代码操作，用于自动插入其已知诊断的检查。

![IMG](/mlir-lsp-server/diagnostics_action.gif)

#### 代码补全

语言服务器在您键入时提供建议，为方言构造（如属性、操作和类型）、块名称、SSA 值名称、关键字等提供补全。

![IMG](/mlir-lsp-server/code_complete.gif)

#### 交叉引用

交叉引用允许导航 SSA 值（即操作结果和块参数）、[符号](../SymbolsAndSymbolTables.md)和块的使用/定义链。

##### 查找定义

跳转到光标下 IR 实体的定义。以下是几个示例：

- SSA 值

![SSA](/mlir-lsp-server/goto_def_ssa.gif)

- 符号引用

![Symbols](/mlir-lsp-server/goto_def_symbol.gif)

操作的定义还会考虑附加的源位置，允许导航到生成该操作的源文件。

![External Locations](/mlir-lsp-server/goto_def_external.gif)

##### 查找引用

显示光标下 IR 实体的所有引用。

![IMG](/mlir-lsp-server/find_references.gif)

#### 悬停

将鼠标悬停在 IR 实体上以查看有关它的更多信息。显示的确切信息取决于光标下 IR 实体的类型。例如，将鼠标悬停在 `Operation` 上可能会显示其通用格式。

![IMG](/mlir-lsp-server/hover.png)

#### 导航

语言服务器还会将 IR 中符号表的结构通知编辑器。这允许直接跳转到文件中符号（如 `func.func`）的定义。

![IMG](/mlir-lsp-server/navigation.gif)

#### 字节码编辑和检查

语言服务器提供与 MLIR 字节码文件交互的支持，使 IDE 能够以与文本 `.mlir` 文件相同的方式透明地查看和编辑字节码文件。

![IMG](/mlir-lsp-server/bytecode_edit.gif)

## PDLL LSP 语言服务器：`mlir-pdll-lsp-server`

MLIR 以 `mlir-pdll-lsp-server` 工具的形式为 `.pdll` 文本文件提供 LSP 语言服务器的实现。该工具与 PDLL C++ API 交互，以支持丰富的语言查询，例如代码补全和"查找定义"。

### 编译数据库

与 [`clangd`](https://clang.llvm.org/docs/JSONCompilationDatabase.html) 以及其他各种编程语言的语言服务器类似，PDLL 语言服务器依赖编译数据库为 `.pdll` 文件提供构建系统信息。例如，该信息包括该文件可用的包含目录。此数据库允许服务器使用与构建时相同的配置与 `.pdll` 文件交互。

#### 格式

PDLL 编译数据库是一个 YAML 文件，通常命名为 `pdll_compile_commands.yml`，包含一组为各个 `.pdll` 文件提供信息的 `FileInfo` 文档。

示例：

```yaml
--- !FileInfo:
  filepath: "/home/user/llvm/mlir/lib/Dialect/Arith/IR/ArithCanonicalization.pdll"
  includes: "/home/user/llvm/mlir/lib/Dialect/Arith/IR;/home/user/llvm/mlir/include"
```

- filepath: <string> - 文件的绝对路径。
- includes: <string> - 以分号分隔的绝对包含目录列表。

#### 构建系统集成

按照惯例，PDLL 编译数据库应命名为 `pdll_compile_commands.yml` 并放置在构建目录的顶层。使用 CMake 和 `mlir_pdll` 时，通常会自动构建编译数据库并放置在适当位置。

### 功能

本节详细介绍 PDLL 语言服务器提供的几个功能。截图显示在 [VSCode](https://code.visualstudio.com/) 中，但可用的确切功能集取决于您的编辑器客户端。

[pdll features]: #

#### 诊断

语言服务器在您键入时主动运行验证，并就地显示任何生成的诊断。

![IMG](/mlir-pdll-lsp-server/diagnostics.png)

#### 代码补全和签名帮助

语言服务器根据当前上下文中可用的约束、重写、方言、操作等，在您键入时提供建议。服务器还在您填写约束和重写调用、操作等时提供有关其结构的信息。

![IMG](/mlir-pdll-lsp-server/code_complete.gif)

#### 交叉引用

交叉引用允许导航代码库。

##### 查找定义

跳转到光标下符号的定义：

![IMG](/mlir-pdll-lsp-server/goto_def.gif)

如果 ODS 信息可用，我们还可以跳转到操作名称等的定义：

![IMG](/mlir-pdll-lsp-server/goto_def_ods.gif)

##### 查找引用

显示光标下符号的所有引用。

![IMG](/mlir-pdll-lsp-server/find_references.gif)

#### 悬停

将鼠标悬停在符号上以查看有关它的更多信息，例如其类型、文档等。

![IMG](/mlir-pdll-lsp-server/hover.png)

如果 ODS 信息可用，我们还可以直接显示来自操作定义的信息：

![IMG](/mlir-pdll-lsp-server/hover_ods.png)

#### 导航

语言服务器还会将 IR 中符号的结构通知编辑器。

![IMG](/mlir-pdll-lsp-server/navigation.gif)

#### 查看中间输出

语言服务器提供对各种中间编译阶段的内省支持，例如 AST、包含生成 PDL 的 `.mlir` 以及生成的 C++ 胶水代码。这是一个自定义 LSP 扩展，并非所有 IDE 客户端都必然提供。

![IMG](/mlir-pdll-lsp-server/view_output.gif)

#### 内嵌提示

语言服务器在源代码中内联提供额外信息。编辑器通常使用散布在代码中的只读虚拟文本片段来渲染此信息。提示可能显示：

- 局部变量的类型
- 操作数和结果组的名称
- 约束和重写参数

![IMG](/mlir-pdll-lsp-server/inlay_hints.png)

## TableGen LSP 语言服务器：`tblgen-lsp-server`

MLIR 以 `tblgen-lsp-server` 工具的形式为 `.td` 文本文件提供 LSP 语言服务器的实现。该工具与 TableGen C++ API 交互，以支持丰富的语言查询，例如"查找定义"。

### 编译数据库

与 [`clangd`](https://clang.llvm.org/docs/JSONCompilationDatabase.html) 以及其他各种编程语言的语言服务器类似，TableGen 语言服务器依赖编译数据库为 `.td` 文件提供构建系统信息。例如，该信息包括该文件可用的包含目录。此数据库允许服务器使用与构建时相同的配置与 `.td` 文件交互。

#### 格式

TableGen 编译数据库是一个 YAML 文件，通常命名为 `tablegen_compile_commands.yml`，包含一组为各个 `.td` 文件提供信息的 `FileInfo` 文档。

示例：

```yaml
--- !FileInfo:
  filepath: "/home/user/llvm/mlir/lib/Dialect/Arith/IR/ArithCanonicalization.td"
  includes: "/home/user/llvm/mlir/lib/Dialect/Arith/IR;/home/user/llvm/mlir/include"
```

- filepath: <string> - 文件的绝对路径。
- includes: <string> - 以分号分隔的绝对包含目录列表。

#### 构建系统集成

按照惯例，TableGen 编译数据库应命名为 `tablegen_compile_commands.yml` 并放置在构建目录的顶层。使用 CMake 和 `mlir_tablegen` 时，通常会自动构建编译数据库并放置在适当位置。

### 功能

本节详细介绍 TableGen 语言服务器提供的几个功能。截图显示在 [VSCode](https://code.visualstudio.com/) 中，但可用的确切功能集取决于您的编辑器客户端。

[tablegen features]: #

#### 诊断

语言服务器在您键入时主动运行验证，并就地显示任何生成的诊断。

![IMG](/tblgen-lsp-server/diagnostics.png)

#### 交叉引用

交叉引用允许导航代码库。

##### 查找定义

跳转到光标下符号的定义：

![IMG](/tblgen-lsp-server/goto_def.gif)

##### 查找引用

显示光标下符号的所有引用。

![IMG](/tblgen-lsp-server/find_references.gif)

#### 悬停

将鼠标悬停在符号上以查看有关它的更多信息，例如其类型、文档等。

![IMG](/tblgen-lsp-server/hover_def.png)

将鼠标悬停在被覆盖的字段上还会显示来自基础值的文档等信息：

![IMG](/tblgen-lsp-server/hover_field.png)

## 语言服务器设计

MLIR 提供的各种语言服务器的设计实际上是相同的，主要由三个不同组件组成：

- 通信和传输（通过 JSON-RPC）
- 语言服务器协议
- 语言特定服务器

![Index Map Example](/includes/img/mlir-lsp-server-server_diagram.svg)

### 通信和传输

语言服务器（如 `mlir-lsp-server`）通过 stdin/stdout 上的 JSON-RPC 与语言客户端通信。在代码中，这是 `JSONTransport` 类。该类对语言服务器协议一无所知，它只知道 JSON-RPC 消息进来和 JSON-RPC 消息出去。传入和传出 LSP 消息的处理留给 `MessageHandler` 类。该类将传入消息路由到`语言服务器协议`层中的处理器进行解释，并将传出消息打包以供传输。该类对 LSP 的了解也有限，只有关于三类主要消息的信息：通知、调用和回复。

### 语言服务器协议

`LSPServer` 处理更精细的 LSP 细节的解释。该类注册 LSP 消息的处理器，然后转发给[`语言特定服务器`](#language-specific-server)进行处理。该组件的目的是保存从 LSP 世界到语言特定世界（例如 MLIR、PDLL 等）通信时必要的所有胶水代码。在大多数情况下，LSP 消息处理器会直接转发到`语言特定服务器`。然而，在某些情况下，两者之间的阻抗不匹配需要更复杂的胶水代码。

### 语言特定服务器

语言特定服务器（如 `MLIRServer` 或 `PDLLServer`）为特定语言的所有 LSP 查询提供内部实现。这些类直接与语言的 C++ API 交互，包括解析文本文件、解释定义/引用信息等。

## 编辑器插件

LSP 语言插件适用于许多流行的编辑器，原则上 MLIR 提供的语言服务器应与其中任何一个配合使用，但功能集和接口可能有所不同。以下是一些已知可以工作的插件：

### Visual Studio Code

[MLIR 扩展](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-mlir)
为 [MLIR](https://mlir.llvm.org/) 相关语言提供语言 IDE 功能：[MLIR](#mlir---mlir-textual-assembly-format)、[PDLL](#pdll---mlir-pdll-pattern-files) 和 [TableGen](#td---tablegen-files)

#### `.mlir` - MLIR 文本汇编格式：

MLIR 扩展为 [MLIR 文本汇编格式](https://mlir.llvm.org/docs/LangRef/) 添加语言支持：

##### 功能

- `.mlir` 文件和 `mlir` Markdown 块的语法高亮
- 跳转到定义和交叉引用
- 将鼠标悬停在 IR 实体上时显示详细信息
- 符号和符号表的大纲和导航
- 代码补全
- 实时解析器和验证器诊断

[mlir-vscode features]: #

##### 设置

###### `mlir-lsp-server`

各种 `.mlir` 语言功能需要 [`mlir-lsp-server` 语言服务器](https://mlir.llvm.org/docs/Tools/MLIRLSP/#mlir-lsp-language-server--mlir-lsp-server)。如果在工作区路径中找不到 `mlir-lsp-server`，则必须通过 `mlir.server_path` 设置指定服务器路径。服务器路径可以是绝对路径或工作区内的相对路径。

#### `.pdll` - MLIR PDLL 模式文件：

MLIR 扩展为 [PDLL 模式语言](https://mlir.llvm.org/docs/PDLL/) 添加语言支持。

##### 功能

- `.pdll` 文件和 `pdll` Markdown 块的语法高亮
- 跳转到定义和交叉引用
- 悬停时显示类型和文档
- 代码补全和签名帮助
- 查看中间 AST、MLIR 或 C++ 输出

[pdll-vscode features]: #

##### 设置

###### `mlir-pdll-lsp-server`

各种 `.pdll` 语言功能需要 [`mlir-pdll-lsp-server` 语言服务器](https://mlir.llvm.org/docs/Tools/MLIRLSP/#pdll-lsp-language-server--mlir-pdll-lsp-server)。如果在工作区路径中找不到 `mlir-pdll-lsp-server`，则必须通过 `mlir.pdll_server_path` 设置指定服务器路径。服务器路径可以是绝对路径或工作区内的相对路径。

###### 项目设置

为了正确理解和与 `.pdll` 文件交互，语言服务器必须了解项目的构建方式（编译标志）。应提供与您的项目相关的 [`pdll_compile_commands.yml` 文件](https://mlir.llvm.org/docs/Tools/MLIRLSP/#compilation-database)，以确保文件被正确处理。这些文件通常可以由构建系统生成，服务器将尝试在您的 `build/` 目录中查找它们。如果不可用或位于唯一位置，可以通过 `mlir.pdll_compilation_databases` 设置指定其他 `pdll_compile_commands.yml` 文件。这些数据库的路径可以是绝对路径或工作区内的相对路径。

#### `.td` - TableGen 文件：

MLIR 扩展为 [TableGen 语言](https://llvm.org/docs/TableGen/ProgRef.html) 添加语言支持。

##### 功能

- `.td` 文件和 `tablegen` Markdown 块的语法高亮
- 跳转到定义和交叉引用
- 悬停时显示类型和文档

[tablegen-vscode features]: #

##### 设置

###### `tblgen-lsp-server`

各种 `.td` 语言功能需要 [`tblgen-lsp-server` 语言服务器](https://mlir.llvm.org/docs/Tools/MLIRLSP/#tablegen-lsp-language-server--tblgen-lsp-server)。如果在工作区路径中找不到 `tblgen-lsp-server`，则必须通过 `mlir.tablegen_server_path` 设置指定服务器路径。服务器路径可以是绝对路径或工作区内的相对路径。

###### 项目设置

为了正确理解和与 `.td` 文件交互，语言服务器必须了解项目的构建方式（编译标志）。应提供与您的项目相关的 [`tablegen_compile_commands.yml` 文件](https://mlir.llvm.org/docs/Tools/MLIRLSP/#compilation-database-1)，以确保文件被正确处理。这些文件通常可以由构建系统生成，服务器将尝试在您的 `build/` 目录中查找它们。如果不可用或位于唯一位置，可以通过 `mlir.tablegen_compilation_databases` 设置指定其他 `tablegen_compile_commands.yml` 文件。这些数据库的路径可以是绝对路径或工作区内的相对路径。

#### 贡献

该扩展在 [LLVM monorepo](https://github.com/llvm/llvm-project) 中积极开发，位于 [`mlir/utils/vscode`](https://github.com/llvm/llvm-project/tree/main/mlir/utils/vscode)。因此，贡献应遵循[正常的 LLVM 指南](https://llvm.org/docs/Contributing.html)，代码审查提交至 [GitHub](https://llvm.org/docs/Contributing.html#how-to-submit-a-patch)。

在 LLVM monorepo 中开发或部署此扩展时，需要一些额外的设置步骤：

- 将 `mlir/utils/textmate/mlir.json` 复制到扩展目录并重命名为 `grammar.json`。
- 将 `llvm/utils/textmate/tablegen.json` 复制到扩展目录并重命名为 `tablegen-grammar.json`。
- 将 `https://mlir.llvm.org//LogoAssets/logo/PNG/full_color/mlir-identity-03.png` 复制到扩展目录并重命名为 `icon.png`。

在为扩展贡献时，请遵循现有代码风格，我们建议在发送补丁之前运行 `npm run format`。
