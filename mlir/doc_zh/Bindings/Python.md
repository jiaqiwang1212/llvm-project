# MLIR Python 绑定

**当前状态**：正在开发中，默认未启用

[TOC]

## 构建

### 前置条件

*   安装[非 EOL](https://devguide.python.org/versions/) 的 Python3
*   按照 `mlir/python/requirements.txt` 中的说明安装 Python 依赖项

### CMake 变量

*   **`MLIR_ENABLE_BINDINGS_PYTHON`**`:BOOL`

    启用 Python 绑定的构建。默认值为 `OFF`。

*   **`Python3_EXECUTABLE`**:`STRING`

    指定用于 LLVM 构建的 `python` 可执行文件，包括用于确定 Python 绑定的头文件/链接标志。在有多个 Python 实现的系统上，强烈建议将此项明确设置为首选的 `python3` 可执行文件。

*   **`CMAKE_C_VISIBILITY_PRESET`**: `STRING`
*   **`CMAKE_CXX_VISIBILITY_PRESET`**: `STRING`
*   **`CMAKE_VISIBILITY_INLINES_HIDDEN`**: `BOOL`

    如果最终构建的包将在多个绑定包同时使用的场景下使用（即在同一 Python 解释器会话中加载多个绑定包），**强烈**建议将这些选项分别设置为 `hidden`、`hidden` 和 `ON`。否则可能导致符号解析不正确/模糊；其症状表现为如下 `LLVM ERROR`：
    ```
    LLVM ERROR: ... unregistered/uninitialized dialect/type/pass ...`
    ```

*   **`MLIR_BINDINGS_PYTHON_NB_DOMAIN`**: `STRING`

    编译扩展所在的 nanobind（及 MLIR）域。
    这决定了该包是否与其他绑定包共享 nanobind 类型。
    每个项目（以及拥有多个绑定包的项目中的每组绑定）应保持唯一。
    也可以显式传递给 `add_mlir_python_modules`。

### 推荐的开发实践

建议使用 Python 虚拟环境。有多种方式可以实现，一般推荐以下方式之一：

```shell
# 确认你的 'python' 是预期的版本。注意在多 Python 系统上，
# 可能有版本后缀；在 python2 和 python3 并存的 Linux 和 macOS 上，
# 你可能还需要使用 `python3`。
which python
python -m venv ~/.venv/mlirdev
source ~/.venv/mlirdev/bin/activate
```

或者，如果你的系统安装了 uv，也可以使用以下命令创建相同的环境（本示例以 Python 3.12 工具链为目标）：

```shell
uv venv ~/.venv/mlirdev --seed -p 3.12
source ~/.venv/mlirdev/bin/activate
```

可以按需更改 Python 版本（`-p` 标志）——如果请求的 Python 解释器不在系统上，uv 将尝试下载，除非指定了 `--no-python-downloads` 选项。
关于如何安装 uv，请参阅官方文档：
https://docs.astral.sh/uv/getting-started/installation/

```shell
# 注意很多 LTS 发行版自带的 pip 版本过旧，无法下载某些平台的最新二进制包。
# 可以通过 `python -m pip --version` 查看 pip 版本，对于 Linux 系统，
# 建议与此处的最低版本进行对比：https://github.com/pypa/manylinux
# 建议升级 pip：
python -m pip install --upgrade pip

# 现在 `python` 命令将解析到你的虚拟环境，包将安装在那里。
python -m pip install -r mlir/python/requirements.txt

# 在 uv 生成的虚拟环境中，可以改为运行：
uv pip install -r mlir/python/requirements.txt

# 现在用 `cmake`、`ninja` 等运行你的构建命令。

# 运行 mlir 测试。例如，仅使用 ninja 运行 Python 绑定测试：
ninja check-mlir-python
```

对于交互式使用，只需将 `build/` 目录中的
`tools/mlir/python_packages/mlir_core/` 目录添加到
`PYTHONPATH` 即可。通常方式为：

```shell
export PYTHONPATH=$(cd build && pwd)/tools/mlir/python_packages/mlir_core
```

注意，如果你已经安装（例如通过 `ninja install` 等），所有已启用项目的 Python 包将位于安装树的 `python_packages/` 下（例如 `python_packages/mlir_core`）。官方发行版使用更专门化的设置进行构建。

## 设计

### 使用场景

MLIR Python 绑定大致有两种主要使用场景：

1.  支持希望通过安装的 LLVM/MLIR 版本直接 `import mlir` 并以纯粹方式使用 API 的用户。

1.  下游集成可能希望将部分 API 纳入其私有命名空间或专门构建的库中，可能与其他 Python 原生部分混合使用。

### 可组合模块

为了支持场景 \#2，Python 绑定被组织成可组合模块，下游集成者可以按需将其纳入并重新导出到自己的命名空间中。这带来了几个设计要点：

*   将 `nb::module` 的构建/填充与 `NB_MODULE` 全局构造函数分离。

*   为仅 C++ 的包装类引入头文件，因为其他相关 C++ 模块需要与其互操作。

*   将依赖可选组件的初始化例程分离到其自己的模块/依赖项中（目前，诸如 `registerAllDialects` 之类的内容属于此类别）。

共享库链接、发行问题等相互关联的问题影响着这些方面。将代码组织成可组合模块（而非单一的 `cpp` 文件）为随时间推移按需解决这些问题提供了灵活性。此外，pybind 中所有模板元编程的编译时间与翻译单元中定义的内容数量成正比。拆分成多个翻译单元可以显著改善具有大量 API 表面的接口的编译时间。

### 子模块

通常，C++ 代码库将大多数内容命名空间化到 `mlir` 命名空间中。然而，为了模块化并使 Python 绑定更易于理解，定义了大致映射到 MLIR 中功能单元目录结构的子包。

示例：

*   `mlir.ir`
*   `mlir.passes`（`pass` 是保留字 :( ）
*   `mlir.dialect`
*   `mlir.execution_engine`（除了命名空间之外，将"笨重"/可选的部分隔离也很重要）

此外，暗示可选依赖项的初始化函数应放在带下划线前缀（表示私有）的模块中，如 `_init`，并单独链接。这允许下游集成者完全自定义"开箱即用"的内容，涵盖方言注册、pass 注册等。

### 加载器

LLVM/MLIR 是一个重量级的 Python 原生项目，可能与其他重量级的原生扩展共存。因此，原生扩展（即 `.so`/`.pyd`/`.dylib`）被导出为名义上私有的顶级符号（`_mlir`），而在 `mlir/_cext_loader.py` 及其兄弟文件中提供少量 Python 代码来加载并重新导出它。这种分离提供了在共享库加载到 Python 运行时*之前*准备环境的代码存放处，也提供了一次性初始化代码可以在模块构造函数之外被调用的地方。

建议尽量避免使用 `__init__.py` 文件，直到到达表示离散组件的叶包为止。需要记住的规则是：`__init__.py` 文件的存在会阻止在该级别或以下的命名空间中将任何内容拆分到不同目录、部署包、wheel 等中。

有关更多信息和建议，请参阅文档：
https://packaging.python.org/guides/packaging-namespace-packages/

### 使用 C-API

Python API 应尽可能在 C-API 之上构建。特别是对于核心的、与方言无关的部分，这种绑定方式能够实现若跨越 C++ ABI 边界则难以或无法实现的打包决策。此外，以这种方式分层可以避开将基于 RTTI 的模块（pybind 派生物就是如此）与非 RTTI 的多态 C++ 代码（LLVM 的默认编译模式）结合时出现的一些非常棘手的问题。

### 核心 IR 中的所有权

核心 IR 中有几种顶级类型由其 Python 端引用强拥有：

*   `PyContext` (`mlir.ir.Context`)
*   `PyModule` (`mlir.ir.Module`)
*   `PyOperation` (`mlir.ir.Operation`) — 但有注意事项

所有其他对象均为从属对象。所有对象都维护一个指向最近包含的顶级对象的反向引用（保持活跃）。此外，从属对象分为两类：a) 唯一化（在上下文的生命周期内存活）和 b) 可变。可变对象需要额外的机制来跟踪支持其 Python 对象的 C++ 实例何时不再有效（通常由于 IR 的某些特定变更、删除或批量操作）。

### 核心 IR 中的可选性与参数排序

以下类型支持作为上下文管理器绑定到当前线程：

*   `PyLocation` (`loc: mlir.ir.Location = None`)
*   `PyInsertionPoint` (`ip: mlir.ir.InsertionPoint = None`)
*   `PyMlirContext` (`context: mlir.ir.Context = None`)

为了支持函数参数的可组合性，当这些类型作为参数出现时，应始终是最后的参数，并按上述顺序排列，使用给定的名称（这通常是在特殊情况下需要显式表达的顺序），并根据需要出现。每个都应携带默认值 `py::none()`，并使用手动或自动转换，以从显式值或线程上下文管理器中的值进行解析（即 `DefaultingPyMlirContext` 或 `DefaultingPyLocation`）。

其原因在于，在 Python 中，*右侧*的尾部关键字参数最具可组合性，支持 kwargs 透传、默认值等多种策略。保持函数签名的可组合性，增加了在无需大量异常样板代码的情况下构建有趣的 DSL 和高层 API 的可能性。

一致使用此规范，可以实现一种 IR 构建风格：很少需要显式使用上下文、位置或插入点，但在需要额外控制时可以自由使用。

#### 操作层次结构

如前所述，`PyOperation` 是特殊的，因为它可以处于顶级或从属状态。生命周期是单向的：操作可以以独立（顶级）状态创建，一旦添加到另一个操作中，便在其剩余生命周期内保持从属状态。当考虑到操作被添加到仍然独立的传递父级的构建场景时，情况会更加复杂，这需要在此类过渡点进行进一步的记录（即所有此类添加的子级最初以其最外层独立操作为父级添加到 IR 中，但一旦将其添加到已附加的操作中，它们需要被重新指向包含模块）。

由于有效性和父级记录的需要，`PyOperation` 是区域（regions）和块（blocks）的所有者。操作也是唯一允许处于独立状态的实体。

**注意**：多个 `PyOperation` 对象（即 Python 对象本身）可以别名同一个 `mlir::Operation`。
这意味着，例如，如果 `py_op1` 和 `py_op2` 都包装了同一个 `mlir::Operation op`，
而你以某种方式转换了 `op`（例如，对 `op` 运行了一个 pass），那么通过 `py_op1` 或 `py_op2` 遍历 MLIR AST 都会反映相同的 MLIR AST。这是完全安全且支持的。不支持的是：在存在多个包装该操作的 Python 对象时使某个操作无效，**然后操作这些包装器**。
例如，如果 `py_op1` 和 `py_op2` 包装了根 `py_op3` 下的同一个操作，然后 `py_op3` 被转换使得 `py_op1`、`py_op2` 引用的操作被删除，则 `py_op1`、`py_op2` 在某种意义上变成"未定义"的；以任何方式操作它们都是"正式禁止的"。注意，这也适用于 `SymbolTable` 的变更，在本讨论中，这被视为对根 `SymbolTable` 支持操作的转换。打个比方，这类似于 STL 容器迭代器在容器本身更改后如何失效。"最佳实践"建议是将代码结构化为：

1. 首先，查询/操作各种 Python 包装对象 `py_op1`、`py_op2`、`py_op3` 等；
2. 其次，通过单个根对象转换 AST/删除操作等；
3. 使所有已查询的节点无效（例如，使用 `op._set_invalid()`）。

理想情况下，这应在函数体中完成，使步骤（3）对应函数末尾，不存在 Python 包装对象泄漏/存活时间超过必要时长的风险。总之，你应基于嵌套来限定变更范围，即先更改叶节点，再向上层级进行，仅在极少数情况下在修改父操作后查询嵌套操作。

C/C++ API 允许区域/块也处于独立状态，但在此 API 中消除这种可能性大大简化了所有权模型，使区域/块完全依赖于其拥有操作进行记录。Python 中 `Region`/`Block` 实例与底层 `MlirRegion`/`MlirBlock` 的别名被认为是无害的，这些对象在上下文中不会被 interned（与操作不同）。

如果我们将来想重新引入独立的区域/块，可以通过新的"DetachedRegion"类或类似方式实现，同时也可以避免记录的复杂性。按照目前的方式，我们可以避免为区域和块维护全局活跃列表。根据变更与其 Python 对等对象的交互情况，我们可能在某个时刻需要一个操作本地的列表，具体待定。届时可以轻松解决这个问题。

## 用户级 API

### 上下文管理

绑定依赖 Python
[上下文管理器](https://docs.python.org/3/reference/datamodel.html#context-managers)
（`with` 语句）通过省略 MLIR 上下文、操作插入点和位置等重复参数来简化 IR 对象的创建和处理。上下文管理器为以下上下文及同一线程中的所有绑定调用设置要使用的默认对象。此默认值可以通过专用关键字参数的特定调用来覆盖。

#### MLIR 上下文

MLIR 上下文是拥有属性和类型的顶级实体，几乎从所有 IR 构造中引用。上下文还在 C++ 级别提供线程安全性。在 Python 绑定中，MLIR 上下文也是一个 Python 上下文管理器，可以这样写：

```python
from mlir.ir import Context, Module

with Context() as ctx:
  # 使用 `ctx` 作为上下文进行 IR 构建。

  # 例如，从字符串解析 MLIR 模块需要上下文。
  Module.parse("builtin.module {}")
```

引用上下文的 IR 对象通常通过 `.context` 属性提供对其的访问。大多数 IR 构建函数期望以某种形式提供上下文。对于属性和类型，上下文可以从包含的属性或类型中提取。对于操作，上下文系统地从位置（见下文）中提取。当无法从任何参数中提取上下文时，绑定 API 期望（关键字）参数 `context`。如果未提供或设置为 `None`（默认值），将从绑定在当前线程中维护的隐式上下文栈（由上下文管理器更新）中查找。如果没有外围上下文，将引发错误。

注意，可以在 `with` 语句内外手动指定 MLIR 上下文：

```python
from mlir.ir import Context, Module

standalone_ctx = Context()
with Context() as managed_ctx:
  # 在 managed_ctx 中解析模块。
  Module.parse("...")

  # 在 standalone_ctx 中解析模块（覆盖上下文管理器）。
  Module.parse("...", context=standalone_ctx)

# 不使用上下文管理器解析模块。
Module.parse("...", context=standalone_ctx)
```

只要有 IR 对象引用上下文，上下文对象就保持活跃。

#### 插入点与位置

构建 MLIR 操作时，需要两条信息：

-   一个*插入点*，指示操作将在 IR 区域/块/操作结构中的哪个位置创建（通常在另一个操作之前或之后，或某个块的末尾）；它可能缺失，在这种情况下操作以*独立*状态创建；
-   一个*位置*，包含关于操作来源的用户可理解信息（例如，文件/行/列信息），必须始终提供，因为它携带对 MLIR 上下文的引用。

两者都可以使用上下文管理器提供，也可以作为操作构造函数中的关键字参数显式提供。它们也可以作为关键字参数 `ip` 和 `loc` 在上下文管理器内外提供。

```python
from mlir.ir import Context, InsertionPoint, Location, Module, Operation

with Context() as ctx:
  module = Module.create()

  # 准备将操作插入模块主体，并指示这些操作来源于
  # "f.mlir" 文件的给定行和列。
  with InsertionPoint(module.body), Location.file("f.mlir", line=42, col=1):
    # 该操作将在模块主体末尾插入，并具有由上下文管理器设置的位置。
    Operation(<...>)

    # 该操作将在模块末尾（紧接在前一个构建的操作之后）插入，
    # 并具有作为关键字参数提供的位置。
    Operation(<...>, loc=Location.file("g.mlir", line=1, col=10))

    # 该操作将在块的*开头*插入，而不是末尾。
    Operation(<...>, ip=InsertionPoint.at_block_begin(module.body))
```

注意，`Location` 需要一个 MLIR 上下文来构建。它可以获取当前线程中某个外围上下文管理器设置的上下文，或接受一个显式参数：

```python
from mlir.ir import Context, Location

# 在同一个 `with` 语句中创建上下文和该上下文中的位置。
with Context() as ctx, Location.file("f.mlir", line=42, col=1, context=ctx):
  pass
```

位置由上下文拥有，只要在 Python 代码中某处（间接）引用它们，就保持活跃。

与位置不同，插入点在操作构建期间可以不指定（或等效地设置为 `None` 或 `False`）。在这种情况下，操作以*独立*状态创建，即不会添加到另一个操作的区域中，由调用者拥有。顶级操作（包含 IR 的操作，如模块）通常就是这种情况。操作中包含的区域、块和值反向指向该操作并使其保持活跃。

### 检查 IR 对象

检查 IR 是 Python 绑定设计用于的主要任务之一。可以遍历 IR 操作/区域/块结构，并检查其各方面，如操作属性和值类型。

#### 操作、区域和块

操作表示为：

-   通用 `Operation` 类，特别适用于对未注册操作的通用处理；或
-   `OpView` 的特定子类，为操作属性提供语义更丰富的访问器。

给定一个 `OpView` 子类，可以通过其 `.operation` 属性获取 `Operation`。给定一个 `Operation`，可以通过其 `.opview` 属性获取对应的 `OpView`，*前提是*对应的类已设置好。这通常意味着其方言的 Python 模块已被加载。默认情况下，在遍历 IR 树时产生 `OpView` 版本。

可以通过 Python 的 `isinstance` 函数检查操作是否具有特定类型：

```python
operation = <...>
opview = <...>
if isinstance(operation.opview, mydialect.MyOp):
  pass
if isinstance(opview, mydialect.MyOp):
  pass
```

可以使用操作的属性检查操作的组件。

-   `attributes` 是操作属性的集合。它可以像字典和序列一样被下标索引，例如，`operation.attributes["value"]` 和 `operation.attributes[0]` 都可以工作。将 `attributes` 属性作为序列迭代时，不保证属性的遍历顺序。
-   `operands` 是操作操作数的序列集合。
-   `results` 是操作结果的序列集合。
-   `regions` 是附加到操作的区域的序列集合。

`operands` 和 `results` 产生的对象具有 `.types` 属性，包含对应值类型的序列集合。

```python
from mlir.ir import Operation

operation1 = <...>
operation2 = <...>
if operation1.results.types == operation2.operand.types:
  pass
```

特定操作的 `OpView` 子类可能为操作属性提供更简洁的访问器。例如，命名属性、操作数和结果通常可以作为 `OpView` 子类的同名属性访问，例如用 `operation.const_value` 代替 `operation.attributes["const_value"]`。如果该名称是 Python 保留关键字，则在其后加下划线。

操作本身是可迭代的，按顺序提供对附加区域的访问：

```python
from mlir.ir import Operation

operation = <...>
for region in operation:
  do_something_with_region(region)
```

区域在概念上是块的序列。因此，`Region` 类的对象是可迭代的，提供对块的访问。也可以使用 `.blocks` 属性。

```python
# 区域是直接可迭代的，可访问块。
for block1, block2 in zip(operation.regions[0], operation.regions[0].blocks)
  assert block1 == block2
```

块包含一系列操作，并有几个附加属性。`Block` 类的对象是可迭代的，提供对块中包含的操作的访问。`.operations` 属性也同样如此。块还有一个参数列表，可以通过 `.arguments` 属性作为序列集合访问。

在 Python 绑定中，块和区域属于父操作，并使其保持活跃。可以使用 `.owner` 属性访问该操作。

#### 属性和类型

属性和类型是（大多数情况下）不可变的上下文拥有对象。它们表示为：

-   支持打印和比较的不透明 `Attribute` 或 `Type` 对象；或
-   其具体子类，可访问属性或类型的属性。

给定一个 `Attribute` 或 `Type` 对象，可以使用子类的构造函数获取具体子类。如果属性或类型不是预期的子类，可能引发 `ValueError`：

```python
from mlir.ir import Attribute, Type
from mlir.<dialect> import ConcreteAttr, ConcreteType

attribute = <...>
type = <...>
try:
  concrete_attr = ConcreteAttr(attribute)
  concrete_type = ConcreteType(type)
except ValueError as e:
  # 处理不正确的子类。
```

此外，具体属性和类型类提供一个静态 `isinstance` 方法，用于检查不透明 `Attribute` 或 `Type` 类型的对象是否可以向下转换：

```python
from mlir.ir import Attribute, Type
from mlir.<dialect> import ConcreteAttr, ConcreteType

attribute = <...>
type = <...>

# 此处无需处理错误。
if isinstance(attribute, ConcreteAttr):
  concrete_attr = ConcreteAttr(attribute)
if isinstance(type, ConcreteType):
  concrete_type = ConcreteType(type)
```

默认情况下，与操作不同，属性和类型从 IR 遍历中以不透明的 `Attribute` 或 `Type` 返回，需要向下转换。

具体属性和类型类通常将其属性暴露为 Python 只读属性。例如，张量类型的元素类型可以通过 `.element_type` 属性访问。

#### 值

MLIR 根据其定义对象有两种值：块参数和操作结果。值的处理方式与属性和类型类似。它们表示为：

-   通用 `Value` 对象；或
-   具体的 `BlockArgument` 或 `OpResult` 对象。

前者提供所有通用功能，如比较、类型访问和打印。后者提供对定义块或操作及值在其中位置的访问。默认情况下，通用 `Value` 对象从 IR 遍历中返回。向下转换通过具体子类构造函数实现，与属性和类型类似：

```python
from mlir.ir import BlockArgument, OpResult, Value

value = ...

# 将 `concrete` 设置为特定的值子类。
try:
  concrete = BlockArgument(value)
except ValueError:
  # 这里不应再引发另一个 ValueError，因为值要么是块参数要么是操作结果。
  concrete = OpResult(value)
```

#### 接口

MLIR 接口是一种无需了解操作的特定类型、只需了解其某些方面就能与 IR 交互的机制。操作接口作为 Python 类提供，与其 C++ 对应物同名。这些类的对象可以通过以下方式构建：

-   `Operation` 类或任何 `OpView` 子类的对象；在这种情况下，所有接口方法均可用；
-   `OpView` 的子类和一个上下文；在这种情况下，只有*静态*接口方法可用，因为没有关联的操作。

在两种情况下，如果操作类在给定上下文中（或对于操作，在操作所在的上下文中）未实现该接口，则接口的构建会引发 `ValueError`。与属性和类型类似，MLIR 上下文可以由外围上下文管理器设置。

```python
from mlir.ir import Context, InferTypeOpInterface

with Context():
  op = <...>

  # 尝试将操作转换为接口。
  try:
    iface = InferTypeOpInterface(op)
  except ValueError:
    print("操作未实现 InferTypeOpInterface。")
    raise

  # 从 Operation 或 OpView 构建的接口对象上可以使用所有方法。
  iface.someInstanceMethod()

  # 也可以给定 OpView 子类来构建接口对象。
  # 还需要在其中查找接口的上下文。上下文可以显式提供或由外围上下文管理器设置。
  try:
    iface = InferTypeOpInterface(some_dialect.SomeOp)
  except ValueError:
    print("SomeOp 未实现 InferTypeOpInterface。")
    raise

  # 在从类构建的接口对象上调用实例方法会引发 TypeError。
  try:
    iface.someInstanceMethod()
  except TypeError:
    pass

  # 不过仍然可以调用静态接口方法。
  iface.inferOpReturnTypes(<...>)
```

如果接口对象是从 `Operation` 或 `OpView` 构建的，它们分别作为接口对象的 `.operation` 和 `.opview` 属性可用。

目前，Python 绑定中只提供了操作接口的一个子集。属性和类型接口在 Python 绑定中尚不可用。

### 创建 IR 对象

Python 绑定还支持 IR 的创建和操作。

#### 操作、区域和块

可以给定 `Location` 和可选的 `InsertionPoint` 来创建操作。如上所述，对于连续创建的多个操作，使用上下文管理器指定位置和插入点通常更方便。

可以通过对应的 `OpView` 子类的构造函数来创建具体操作。构造函数的通用默认形式接受：

-   操作结果类型的可选序列（`results`）；
-   操作操作数的可选值序列，或产生这些值的另一个操作（`operands`）；
-   操作属性的可选字典（`attributes`）；
-   后继块的可选序列（`successors`）；
-   附加到操作的区域数量（`regions`，默认为 `0`）；
-   包含此操作 `Location` 的 `loc` 关键字参数；如果为 `None`，则使用最近上下文管理器创建的位置，如果没有上下文管理器则引发异常；
-   指示操作将在 IR 中插入位置的 `ip` 关键字参数；如果为 `None`，则使用最近上下文管理器创建的插入点；如果没有外围上下文管理器，操作以独立状态创建。

大多数操作会自定义构造函数，以接受与该操作相关的精简参数列表。例如，零结果操作可以省略 `results` 参数，可以明确地从操作数类型推导结果类型的操作也可以省略。以一个具体示例为例，内置函数操作可以通过提供函数名（字符串）及其参数和结果类型（序列元组）来构建：

```python
from mlir.ir import Context, Module
from mlir.dialects import builtin

with Context():
  module = Module.create()
  with InsertionPoint(module.body), Location.unknown():
    func = func.FuncOp("main", ([], []))
```

另见下文由 ODS 生成的构造函数。

操作也可以使用通用类，基于操作的规范字符串名称通过 `Operation.create` 来构建。它接受操作名称（字符串），必须与 C++ 或 ODS 中操作的规范名称完全匹配，后跟与 `OpView` 默认构造函数相同的参数列表。*不鼓励*使用此形式，它用于通用操作处理。

```python
from mlir.ir import Context, Module
from mlir.dialects import builtin

with Context():
  module = Module.create()
  with InsertionPoint(module.body), Location.unknown():
    # 可以以通用方式创建操作。
    func = Operation.create(
        "func.func", results=[], operands=[],
        attributes={"function_type":TypeAttr.get(FunctionType.get([], []))},
        successors=None, regions=1)
    # 如果有对应的具体 `OpView` 子类，结果将被向下转换到该子类。
    assert isinstance(func, func.FuncOp)
```

区域在 C++ 端构建操作时为操作创建。它们在 Python 中不可构建，也不应在操作外部存在（与支持独立区域的 C++ 不同）。

块可以在给定区域内创建，并使用 `Block` 类的 `create_before()`、`create_after()` 方法或同一类的 `create_at_start()` 静态方法，在同一区域的另一个块之前或之后插入。它们不应在区域外部存在（与支持独立块的 C++ 不同）。

```python
from mlir.ir import Block, Context, Operation

with Context():
  op = Operation.create("generic.op", regions=1)

  # 在区域中创建第一个块。
  entry_block = Block.create_at_start(op.regions[0])

  # 创建更多块。
  other_block = entry_block.create_after()
```

块可以用于创建 `InsertionPoint`，可以指向块的开头或末尾，或其终结符之前。`OpView` 子类通常提供一个 `.body` 属性，可用于构建 `InsertionPoint`。例如，内置的 `Module` 和 `FuncOp` 分别提供 `.body` 和 `.add_entry_blocK()`。

#### 属性和类型

可以给定 `Context` 或另一个已引用上下文的属性或类型对象来创建属性和类型。为了表明它们由上下文拥有，通过调用具体属性或类型类上的静态 `get` 方法来获取它们。这些方法将构建属性或类型所需的数据作为参数，并在无法从其他参数推导上下文时，提供关键字 `context` 参数。

```python
from mlir.ir import Context, F32Type, FloatAttr

# 属性和类型需要访问 MLIR 上下文，可以直接访问或通过另一个上下文拥有的对象访问。
ctx = Context()
f32 = F32Type.get(context=ctx)
pi = FloatAttr.get(f32, 3.14)

# 它们可以使用外围上下文管理器定义的上下文。
with Context():
  f32 = F32Type.get()
  pi = FloatAttr.get(f32, 3.14)
```

某些属性提供额外的构建方法以提高清晰度。

```python
from mlir.ir import Context, IntegerAttr, IntegerType

with Context():
  i8 = IntegerType.get_signless(8)
  IntegerAttr.get(i8, 42)
```

内置属性通常可以从具有相似结构的 Python 类型构建。例如，`ArrayAttr` 可以从属性的序列集合构建，`DictAttr` 可以从字典构建：

```python
from mlir.ir import ArrayAttr, Context, DictAttr, UnitAttr

with Context():
  array = ArrayAttr.get([UnitAttr.get(), UnitAttr.get()])
  dictionary = DictAttr.get({"array": array, "unit": UnitAttr.get()})
```

在操作创建期间使用的属性的自定义构建器可以通过 `register_attribute_builder` 注册。特别地，以下是如何为 `I32Attr` 注册自定义构建器：

```python
@register_attribute_builder("I32Attr")
def _i32Attr(x: int, context: Context):
  return IntegerAttr.get(
        IntegerType.get_signless(32, context=context), x)
```

这允许使用 `I32Attr` 调用操作创建：

```python
foo.Op(30)
```

注册基于 ODS 名称，但注册是通过纯 Python 方法进行的。每个 ODS 属性类型只允许注册一个自定义构建器（例如，I32Attr 只能有一个，可以对应多个底层 IntegerAttr 类型）。

而不是：

```python
foo.Op(IntegerAttr.get(IndexType.get_signless(32, context=context), 30))
```

## 风格

通常，对于 MLIR 的核心部分，Python 绑定应与底层 C++ 结构大体同构。但为了实用性或为了使结果库具有适当的"Python 风格"，会做出一些妥协。

### 属性 vs get\*() 方法

通常倾向于将 `getContext()`、`getName()`、`isEntryBlock()` 等简单方法转换为只读 Python 属性（即 `context`）。这主要是在绑定代码中调用 `def_prop_ro` 而非 `def` 的区别，让 Python 侧使用起来感觉好很多。

例如，推荐：

```c++
m.def_prop_ro("context", ...)
```

而不是：

```c++
m.def("getContext", ...)
```

### **repr** 方法

具有良好打印表示的东西真的很棒 :) 如果有合理的打印形式，将其连接到 `__repr__` 方法（并用 [doctest](#sample-doctest) 验证）可以显著提高生产效率。

### CamelCase vs snake\_case

函数/方法/属性使用 `snake_case` 命名，类使用 `CamelCase` 命名。作为对 Python 风格的机械让步，这可以大大使 API 感觉融入 Python 生态系统中的同类产品。

如有疑问，请选择能与其他
[PEP 8 风格名称](https://pep8.org/#descriptive-naming-styles)
流畅搭配的名称。

### 倾向于使用伪容器

许多核心 IR 构造在实例上直接提供查询计数和 begin/end 迭代器的方法。倾向于将这些提升为专用的伪容器。

例如，对区域内块的直接映射可以这样做：

```python
region = ...

for block in region:

  pass
```

但是，首选以下方式：

```python
region = ...

for block in region.blocks:

  pass

print(len(region.blocks))
print(region.blocks[0])
print(region.blocks[-1])
```

不要泄露 STL 派生的标识符（`front`、`back` 等），而应将它们转换为适当的 `__dunder__` 方法和绑定中的迭代器包装器。

请注意，这可能走向极端，所以要用好判断力。例如，块参数可能看起来像容器，但有用于查找和变更的已定义方法，这些方法在不使语义复杂化的情况下很难正确建模。如果遇到这些情况，只需镜像 C/C++ API 即可。

### 为常见事情提供一站式辅助方法

聚合多个低级实体的一站式辅助方法非常有帮助，在合理范围内是被鼓励的。例如，让 `Context` 拥有 `parse_asm` 或等效方法，以避免需要显式构建 SourceMgr，可以相当方便。一站式辅助方法不必与对底层构造的更完整映射互斥。

## 测试

测试应添加到 `mlir/test/python` 目录中，通常应是具有 lit 运行行的 `.py` 文件。

我们使用基于 `lit` 和 `FileCheck` 的测试：

*   对于生成性测试（产生 IR 的测试），定义一个构建/打印 IR 的 Python 模块，并通过 `FileCheck` 进行管道传输。
*   解析应通过使用原始常量和适当的 `parse_asm` 调用在被测模块内保持自包含。
*   任何文件 I/O 代码都应通过临时文件进行暂存，而不是依赖测试模块外部的文件制件/路径。
*   为方便起见，我们也使用相同的机制测试非生成性 API 交互，按需打印和 `CHECK`。

### FileCheck 测试示例

```python
# RUN: %PYTHON %s | mlir-opt -split-input-file | FileCheck

# TODO: 一旦实际存在，移动到测试实用类。
def print_module(f):
  m = f()
  print("// -----")
  print("// TEST_FUNCTION:", f.__name__)
  print(m.to_asm())
  return f

# CHECK-LABEL: TEST_FUNCTION: create_my_op
@print_module
def create_my_op():
  m = mlir.ir.Module()
  builder = m.new_op_builder()
  # CHECK: mydialect.my_operation ...
  builder.my_op()
  return m
```

## 与 ODS 集成

MLIR Python 绑定与基于 tablegen 的 ODS 系统集成，为 MLIR 方言和操作提供用户友好的包装器。此集成有多个部分，概述如下。大多数细节已省略：请参阅 `mlir.dialects` 下的构建规则和 Python 源码，了解使用此功能的规范方式。

用户负责提供 `{DIALECT_NAMESPACE}.py`（或带有 `__init__.py` 文件的等效目录）作为入口点。

### 生成 `_{DIALECT_NAMESPACE}_ops_gen.py` 包装模块

每个映射到 Python 的方言都需要创建相应的 `_{DIALECT_NAMESPACE}_ops_gen.py` 包装模块。这通过在特定于 Python 绑定的 tablegen 包装器上调用 `mlir-tblgen` 来完成，该包装器包含样板代码和实际的方言特定 `td` 文件。以 `Func`（作为特例被分配命名空间 `func`）为例：

```tablegen
#ifndef PYTHON_BINDINGS_FUNC_OPS
#define PYTHON_BINDINGS_FUNC_OPS

include "mlir/Dialect/Func/IR/FuncOps.td"

#endif // PYTHON_BINDINGS_FUNC_OPS
```

在主仓库中，通过 CMake 函数 `declare_mlir_dialect_python_bindings` 来构建包装器，该函数调用：

```
mlir-tblgen -gen-python-op-bindings -bind-dialect={DIALECT_NAMESPACE} \
    {PYTHON_BINDING_TD_FILE}
```

生成的操作类必须以类似于 C++ 生成代码包含生成头文件的方式包含在 `{DIALECT_NAMESPACE}.py` 文件中：

```python
from ._my_dialect_ops_gen import *
```

### 扩展包装模块的搜索路径

当 Python 绑定需要定位包装模块时，会查询 `dialect_search_path` 并使用它来查找适当命名的模块。对于主仓库，此搜索路径被硬编码为包含 `mlir.dialects` 模块，这是上述构建规则发出包装器的地方。树外的方言可以通过以下调用将其模块添加到搜索路径中：

```python
from mlir.dialects._ods_common import _cext
_cext.globals.append_dialect_search_prefix("myproject.mlir.dialects")
```

### 包装模块代码组织

包装模块 tablegen 发射器输出：

*   一个 `_Dialect` 类（继承 `mlir.ir.Dialect`），带有 `DIALECT_NAMESPACE` 属性。
*   每个操作一个 `{OpName}` 类（继承 `mlir.ir.OpView`）。
*   上述每个类的装饰器，用于向系统注册。

注意：为避免命名冲突，包装模块使用的所有内部名称都以 `_ods_` 为前缀。

每个具体的 `OpView` 子类还定义了几个公共属性：

*   `OPERATION_NAME` 属性，包含完全限定操作名称的 `str`（即 `math.absf`）。
*   如果为操作定义或推断了默认构建器，则有一个 `__init__` 方法。
*   每个操作数或结果的 `@property` getter（对于未命名的操作数/结果使用自动生成的名称）。
*   每个声明属性的 `@property` getter、setter 和 deleter。

它还发射额外的私有属性，用于子类化和自定义（默认情况下省略这些属性，以使用 `OpView` 上的默认值）：

*   `_ODS_REGIONS`：区域数量和类型的规范。目前是一个元组 (min_region_count, has_no_variadic_regions)。注意 API 对此进行一些轻量级验证，但主要目的是捕获足够信息以执行其他默认构建和区域访问器生成。
*   `_ODS_OPERAND_SEGMENTS` 和 `_ODS_RESULT_SEGMENTS`：黑盒值，指示操作数或结果相对于可变参数的结构。由 `OpView._ods_build_default` 用于解码包含列表的操作数和结果列表。

#### 默认构建器

目前，只有一个默认构建器映射到 `__init__` 方法。意图是此 `__init__` 方法代表通常为 C++ 生成的构建器中*最具体*的那个；但目前它只是以下通用形式。

*   每个声明的结果一个参数：
    *   对于单值结果：每个接受一个 `mlir.ir.Type`。
    *   对于可变参数结果：每个接受一个 `List[mlir.ir.Type]`。
*   每个声明的操作数或属性一个参数：
    *   对于单值操作数：每个接受一个 `mlir.ir.Value`。
    *   对于可变参数操作数：每个接受一个 `List[mlir.ir.Value]`。
    *   对于属性，将接受一个 `mlir.ir.Attribute`。
*   尾部的特定用途可选关键字参数：
    *   `loc`：要使用的显式 `mlir.ir.Location`。默认为绑定到线程的位置（即 `with Location.unknown():`），如果未绑定也未指定则报错。
    *   `ip`：要使用的显式 `mlir.ir.InsertionPoint`。默认为绑定到线程的插入点（即 `with InsertionPoint(...):`）。

此外，每个 `OpView` 继承一个 `build_generic` 方法，允许通过（在可变参数情况下嵌套的）`results` 和 `operands` 序列进行构建。这可以用于获取 Python 中否则不支持的操作的默认构建语义，代价是签名非常通用。

#### 扩展生成的 Op 类

如前所述，构建系统为每个具有 Python 绑定的方言生成 Python 源码，如 `_{DIALECT_NAMESPACE}_ops_gen.py`。通常希望以这些生成的类为起点进行进一步自定义，因此提供了一种扩展机制使其容易实现。此机制使用传统继承与 `OpView` 注册相结合。例如，`arith.constant` 的默认构建器

```python
class ConstantOp(_ods_ir.OpView):
  OPERATION_NAME = "arith.constant"

  _ODS_REGIONS = (0, True)

  def __init__(self, value, *, loc=None, ip=None):
    ...
```

期望 `value` 是一个 `TypedAttr`（例如 `IntegerAttr` 或 `FloatAttr`）。
因此，一个自然的扩展是一个接受 MLIR 类型和 Python 值并实例化适当 `TypedAttr` 的构建器：

```python
from typing import Union

from mlir.ir import Type, IntegerAttr, FloatAttr
from mlir.dialects._arith_ops_gen import _Dialect, ConstantOp
from mlir.dialects._ods_common import _cext

@_cext.register_operation(_Dialect, replace=True)
class ConstantOpExt(ConstantOp):
    def __init__(
        self, result: Type, value: Union[int, float], *, loc=None, ip=None
    ):
        if isinstance(value, int):
            super().__init__(IntegerAttr.get(result, value), loc=loc, ip=ip)
        elif isinstance(value, float):
            super().__init__(FloatAttr.get(result, value), loc=loc, ip=ip)
        else:
            raise NotImplementedError(f"Building `arith.constant` not supported for {result=} {value=}")
```

这允许像这样构建 `arith.constant` 的实例：

```python
from mlir.ir import F32Type

a = ConstantOpExt(F32Type.get(), 42.42)
b = ConstantOpExt(IntegerType.get_signless(32), 42)
```

注意此示例中扩展机制的三个关键方面：

1. `ConstantOpExt` 直接继承自生成的 `ConstantOp`；
2. 在这个最简单的情况下，只需调用超类的初始化器，即 `super().__init__(...)`；
3. 为了将 `ConstantOpExt` 注册为 `mlir.ir.Operation.opview` 返回的首选 `OpView`（参见[操作、区域和块](#operations-regions-and-blocks)），需要用 `@_cext.register_operation(_Dialect, replace=True)` 装饰该类，**其中必须使用 `replace=True`**。

在某些更复杂的情况下，可能需要通过 `OpView.build_generic` 显式构建 `OpView`（参见[默认构建器](#default-builder)），就像生成的构建器所做的那样。
即，我们必须调用 `OpView.build_generic` **并将结果传递给 `OpView.__init__`**，但小问题是后者已经被生成的构建器重写了。
因此，我们必须调用超类的超类（"祖父类"）的方法；例如：

```python
from mlir.dialects._scf_ops_gen import _Dialect, ForOp
from mlir.dialects._ods_common import _cext

@_cext.register_operation(_Dialect, replace=True)
class ForOpExt(ForOp):
    def __init__(self, lower_bound, upper_bound, step, iter_args, *, loc=None, ip=None):
        ...
        super(ForOp, self).__init__(self.build_generic(...))
```

其中 `OpView.__init__` 通过 `super(ForOp, self).__init__` 调用。
注意，还有其他替代方式来实现这一点（例如，显式写出 `OpView.__init__`）；请参阅 Python 继承的相关讨论。

## 为方言提供 Python 绑定

Python 绑定设计用于支持 MLIR 的开放方言生态系统。方言可以作为 `mlir.dialects` 的子模块暴露给 Python，并与其余绑定互操作。对于只包含操作的方言，只需为这些操作提供 Python API 即可。注意大多数样板 API 可以从 ODS 生成。对于包含属性和类型的方言，由于没有通用机制来创建属性和类型，因此需要通过 C API 来处理。Pass 需要在上下文中注册才能在文本指定的 pass 管理器中使用，这可以在 Python 模块加载时完成。其他功能可以通过暴露相关 C API 并在其上构建 Python API 来提供，类似于属性和类型。


### 操作

方言操作通过用特定操作的构建函数和属性包装通用 `mlir.ir.Operation` 类来在 Python 中提供。因此，无需为它们实现单独的 C API。对于在 ODS 中定义的操作，`mlir-tblgen -gen-python-op-bindings -bind-dialect=<dialect-namespace>` 从声明式描述生成 Python API。
只需创建一个新的 `.td` 文件，包含原始 ODS 定义，并将其用作 `mlir-tblgen` 调用的源即可。
此类 `.td` 文件位于
[`python/mlir/dialects/`](https://github.com/llvm/llvm-project/tree/main/mlir/python/mlir/dialects)。
`mlir-tblgen` 的结果预期按惯例生成名为 `_<dialect-namespace>_ops_gen.py` 的文件。生成的操作类可以如前所述进行扩展。MLIR 提供 [CMake 函数](https://github.com/llvm/llvm-project/blob/main/mlir/cmake/modules/AddMLIRPython.cmake)来自动化此类文件的生成。最后，必须创建 `python/mlir/dialects/<dialect-namespace>.py` 或 `python/mlir/dialects/<dialect-namespace>/__init__.py` 文件，并用从生成文件中的 `import` 填充，以在 Python 中启用 `import mlir.dialects.<dialect-namespace>`。


#### 自定义类型注解

生成的 `__init__` 方法包括操作数和属性参数的类型注解。内置映射涵盖标准 MLIR 类型和属性，但方言可以通过向传递给 `mlir-tblgen -gen-python-op-bindings` 的相同 `.td` 文件添加来自 [`PythonBindings.td`](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Bindings/Python/PythonBindings.td) 的定义来扩展它们：

```tablegen
include "mlir/Bindings/Python/PythonBindings.td"

// 操作数/结果注解：将 ODS 类型约束的 cppClassName 中的 C++ 类型映射到
// Python 类型注解，例如
// `ir.Value` -> `ir.Value[my_dialect.MyTensorType]`。
def : PythonTypeName<"::my_dialect::MyTensorType",
                     "my_dialect.MyTensorType">;

// 属性注解：将 TableGen 属性 def 名称映射到其 AttrBuilder 接受的
// Python 类型，例如
// `Union[Any, ir.Attribute]` -> `Union[my_dialect.MyValue, ir.Attribute]`。
def : PythonAttrType<"MyCustomAttr", "my_dialect.MyValue">;
```

### 属性和类型

方言属性和类型在 Python 中分别作为 `mlir.ir.Attribute` 和 `mlir.ir.Type` 类的子类提供。属性和类型的绑定 API 必须连接到用于构建和检查的相关 C API，这些 API 必须先提供。下游/方言 `Attribute` 和 `Type` 子类的绑定可以使用与"核心"扩展中完全相同的模式来定义：

```c++
#include "mlir/Bindings/Python/IRCore.h"
#include "mlir/Bindings/Python/IRTypes.h"

using namespace mlir::python::MLIR_BINDINGS_PYTHON_DOMAIN;

struct PyTestType : PyConcreteType<PyTestType> {
  // 必需
  static constexpr IsAFunctionTy isaFunction = mlirTypeIsAPythonTestTestType;
  // 可选但推荐
  static constexpr GetTypeIDFunctionTy getTypeIdFunction =
      mlirPythonTestTestTypeGetTypeID;
  static constexpr const char *pyClassName = "TestType";
  using Base::Base;

  static void bindDerived(ClassTy &c) {
    c.def_static("get",
      [](DefaultingPyMlirContext context) {
        return PyTestType(context->getRef(), mlirPythonTestTestTypeGet(context.get()->get()));
      },
      nb::arg("context").none() = nb::none());
  }
};

class PyTestAttr : public PyConcreteAttribute<PyTestAttr> {
public:
  // 必需
  static constexpr IsAFunctionTy isaFunction =
      mlirAttributeIsAPythonTestTestAttribute;
  static constexpr const char *pyClassName = "TestAttr";
  static constexpr GetTypeIDFunctionTy getTypeIdFunction =
      mlirPythonTestTestAttributeGetTypeID;
  using Base::Base;

  static void bindDerived(ClassTy &c) {
    c.def_static(
      "get",
      [](DefaultingPyMlirContext context) {
        return PyTestAttr(context->getRef(), mlirPythonTestTestAttributeGet(context.get()->get()));
      },
      nb::arg("context").none() = nb::none());
  }
};

NB_MODULE(_mlirPythonTestNanobind, m) {
  PyTestAttr::bind(m);
  PyTestType::bind(m);
}
```

更多示例请参见 [`mlir/test/python/lib/PythonTestModuleNanobind.cpp`]。

**注意**：如果你在下游项目中定义此类类型/属性，必须定义 `MLIR_BINDINGS_PYTHON_NB_DOMAIN`（用于确定 `MLIR_BINDINGS_PYTHON_DOMAIN`），使其对你的项目以及项目构建/分发的每组绑定唯一。参见上方的 **CMake 变量**。

方言的属性和类型绑定可以位于 `lib/Bindings/Python/Dialect<Name>.cpp`，应编译到单独的"Python 扩展"库中，放置在 `python/mlir/_mlir_libs` 中，由 Python 在运行时加载。MLIR 提供 [CMake 函数](https://github.com/llvm/llvm-project/blob/main/mlir/cmake/modules/AddMLIRPython.cmake)来自动化此类库的生成。此库应从主方言文件 `python/mlir/dialects/<dialect-namespace>.py` 或 `python/mlir/dialects/<dialect-namespace>/__init__.py` 中 `import`，以确保从 Python 加载方言时类型可用。


### Pass

方言特定的 pass 可以通过在上下文中注册它们并依赖从字符串描述解析 pass 管道的 API，在 Python 中供 pass 管理器使用。这可以通过创建一个新的 nanobind 模块（定义在 `lib/Bindings/Python/<Dialect>Passes.cpp` 中）来实现，该模块调用注册 C API，该 API 必须先提供。对于使用 Tablegen 声明性定义的 pass，`mlir-tblgen -gen-pass-capi-header` 和 `-mlir-tblgen -gen-pass-capi-impl` 自动生成 C API。nanobind 模块必须编译到单独的"Python 扩展"库中，可以从主方言文件 `python/mlir/dialects/<dialect-namespace>.py` 或 `python/mlir/dialects/<dialect-namespace>/__init__.py` 中 `import`，或者从单独的 `passes` 子模块 `python/mlir/dialects/<dialect-namespace>/passes.py` 中 `import`（如果不希望将 pass 与方言一起提供）。

### 其他功能

除 IR 对象或 pass 之外的方言功能，如辅助函数，可以类似于属性和类型暴露给 Python。此功能预期存在 C API，然后可以使用 pybind11 和 [`include/mlir/Bindings/Python/PybindAdaptors.h`](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Bindings/Python/PybindAdaptors.h) 进行包装，或使用 nanobind 和 [`include/mlir/Bindings/Python/NanobindAdaptors.h`](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Bindings/Python/NanobindAdaptors.h) 工具连接到其余 Python API。绑定可以位于单独的模块中，也可以与属性和类型位于同一模块中，并随方言一起加载。


## 在 Python 中扩展 MLIR

MLIR Python 绑定支持在 Python 中定义自定义组件，主要包括方言、pass 和重写模式。以下各节概述了如何实现每一种。

### 方言

`mlir.dialects.ext` 模块支持定义 Python 定义的方言。用户可以通过子类化 `Dialect` 类来定义新方言（例如 `MyDialect`），并通过子类化 `MyDialect.Operation` 在该方言中定义操作。然后可以通过在有效的 `Context` 中调用 `MyDialect.load()` 将方言加载到 MLIR 中。加载后，这些操作可以像其他 `OpView` 子类一样使用。

以下示例展示了如何定义方言以及使用新定义的操作构建 IR。

```python
class MyInt(Dialect, name="myint"):
    pass

class ConstantOp(MyInt.Operation, name="constant"):
    value: IntegerAttr
    cst: Result[IntegerType[32]] = infer_result()

class AddOp(MyInt.Operation, name="add"):
    lhs: Operand[IntegerType[32]]
    rhs: Operand[IntegerType[32]]
    res: Result[IntegerType[32]] = infer_result()

# 下面的代码需要可用的 MLIR 上下文和位置。

MyInt.load()

module = Module.create()
i32 = IntegerType.get(32)
with InsertionPoint(module.body):
    two = ConstantOp(IntegerAttr.get(i32, 2))
    three = ConstantOp(IntegerAttr.get(i32, 3))
    add1 = AddOp(two, three)
    add2 = AddOp(add1, two)
    add3 = AddOp(add2, three)
```

方言也可以通过 Python 中的 IRDL 方言绑定来定义。IRDL 绑定提供了一个 `load_dialects` 函数，可将包含 `irdl.dialect` 操作的 MLIR 模块转换为 MLIR 方言。更多详情，请参阅 [IRDL 方言的文档](../Dialects/IRDL.md)。

### Pass

Pass 可以通过 `PassManager.add` API 定义为 Python 可调用对象。在这种情况下，可调用对象在内部被包装为 `mlir::Pass`，并在调用 `PassManager.run` 时作为 pass 管道的一部分执行。在可调用对象中，`op` 参数表示当前正在转换的操作，而 `pass_` 参数提供对当前 `Pass` 对象的访问，允许执行 `signalPassFailure()` 等操作。可调用对象的生命周期至少延伸到 `PassManager` 被销毁。以下示例代码演示了如何定义 Python pass。

```python
def demo_pass(op, pass_):
    # 对给定的 op 执行某些操作
    pass

pm = PassManager('any')
pm.add(demo_pass)
pm.add('some-cpp-defined-passes')
...
pm.run(some_op)
```

### 重写模式

重写模式可以通过 Python 中 `mlir.rewrite.RewritePatternSet` 的 `add` 方法注册。此方法接受要重写的操作类型和定义*匹配和重写*逻辑的 Python 可调用对象。注意，Python 可调用对象的定义应使重写仅在匹配成功时应用，这对应于返回值可转换为 `False`。

`RewritePatternSet` 可以使用 `freeze` 方法转换为 `FrozenRewritePatternSet`，后者可以通过 `apply_patterns_and_fold_greedily` 使用贪婪模式驱动器应用于操作。以下示例演示了典型用法：

```python
def to_muli(op, rewriter):
    with rewriter.ip:
        new_op = arith.muli(op.lhs, op.rhs, loc=op.location)
    rewriter.replace_op(op, new_op)

patterns = RewritePatternSet()
patterns.add(arith.AddIOp, to_muli)  # 将 arith.addi 重写为 arith.muli
patterns.add(...)
frozen = patterns.freeze()

module = ...
apply_patterns_and_fold_greedily(module, frozen)
```

PDL 方言绑定还支持在 Python 中定义和生成重写模式。`mlir.rewrite.PDLModule` 类接受包含 `pdl.pattern` 操作的模块，可以使用 `freeze` 方法将其转换为 `FrozenRewritePatternSet`。然后可以通过 `apply_patterns_and_fold_greedily` 使用贪婪重写模式驱动器将其冻结集应用于操作。更多信息，请参阅 [PDL 方言文档](/docs/Dialects/PDLOps/)。


## 自由线程（无 GIL）支持

自由线程或无 GIL 支持是指全局解释器锁（GIL）成为可选的 CPython 解释器（>=3.13）。有关该主题的详细信息，请查阅 [PEP-703](https://peps.python.org/pep-0703/) 和此 [Python 自由线程指南](https://py-free-threading.github.io/)。

MLIR Python 绑定在以下意义上与自由线程兼容（存在例外，将在下方讨论）：在**独立**上下文中的多线程操作是安全的。下面是安全使用的示例代码：

```python
# python3.13t example.py
import concurrent.futures

import mlir.dialects.arith as arith
from mlir.ir import Context, Location, Module, IntegerType, InsertionPoint


def func(py_value):
    with Context() as ctx:
        module = Module.create(loc=Location.file("foo.txt", 0, 0))

        dtype = IntegerType.get_signless(64)
        with InsertionPoint(module.body), Location.name("a"):
            arith.constant(dtype, py_value)

    return module


num_workers = 8
with concurrent.futures.ThreadPoolExecutor(max_workers=num_workers) as executor:
    futures = []
    for i in range(num_workers):
        futures.append(executor.submit(func, i))
    assert len(list(f.result() for f in futures)) == num_workers
```

自由线程兼容性的例外情况：
- IR 打印不安全，例如，使用 `PassManager.enable_ir_printing()` 的 `PassManager` 会调用线程不安全的 `llvm::raw_ostream`。
- `Location.emit_error` 的使用不安全（由于线程不安全的 `llvm::raw_ostream`）。
- `Module.dump` 的使用不安全（由于线程不安全的 `llvm::raw_ostream`）。
- `mlir.dialects.transform.interpreter` 的使用不安全。
- `mlir.dialects.gpu` 和 `gpu-module-to-binary` 的使用不安全。
