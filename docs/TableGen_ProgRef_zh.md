# TableGen 程序员参考手册（中文译本）

> 原文：https://llvm.org/docs/TableGen/ProgRef.html  
> LLVM 版本：23.0.0git  
> 翻译日期：2026-04-22

---

## 目录

1. [简介](#1-简介)
   - 1.1 [概念](#11-概念)
2. [源文件](#2-源文件)
3. [词法分析](#3-词法分析)
   - 3.1 [字面量](#31-字面量)
   - 3.2 [标识符](#32-标识符)
   - 3.3 [感叹号运算符](#33-感叹号运算符)
   - 3.4 [包含文件](#34-包含文件)
4. [类型](#4-类型)
5. [值与表达式](#5-值与表达式)
   - 5.1 [简单值](#51-简单值)
   - 5.2 [带后缀的值](#52-带后缀的值)
   - 5.3 [粘贴运算符](#53-粘贴运算符)
6. [语句](#6-语句)
   - 6.1 [class — 定义抽象记录类](#61-class--定义抽象记录类)
   - 6.2 [def — 定义具体记录](#62-def--定义具体记录)
   - 6.3 [示例：类与记录](#63-示例类与记录)
   - 6.4 [let — 覆盖类或记录中的字段](#64-let--覆盖类或记录中的字段)
   - 6.5 [multiclass — 定义多条记录](#65-multiclass--定义多条记录)
   - 6.6 [defm — 调用多类以定义多条记录](#66-defm--调用多类以定义多条记录)
   - 6.7 [示例：多类与 defm](#67-示例多类与-defm)
   - 6.8 [defset — 创建定义集合](#68-defset--创建定义集合)
   - 6.9 [deftype — 定义类型](#69-deftype--定义类型)
   - 6.10 [defvar — 定义变量](#610-defvar--定义变量)
   - 6.11 [foreach — 遍历语句序列](#611-foreach--遍历语句序列)
   - 6.12 [dump — 向标准错误输出打印消息](#612-dump--向标准错误输出打印消息)
   - 6.13 [if — 基于测试选择语句](#613-if--基于测试选择语句)
   - 6.14 [assert — 检查条件是否为真](#614-assert--检查条件是否为真)
7. [附加细节](#7-附加细节)
   - 7.1 [有向无环图（DAG）](#71-有向无环图dag)
   - 7.2 [记录体中的 defvar](#72-记录体中的-defvar)
   - 7.3 [记录的构建过程](#73-记录的构建过程)
8. [将类用作子程序](#8-将类用作子程序)
9. [预处理设施](#9-预处理设施)
10. [附录 A：感叹号运算符](#10-附录-a感叹号运算符)
11. [附录 B：粘贴运算符示例](#11-附录-b粘贴运算符示例)
12. [附录 C：示例记录](#12-附录-c示例记录)

---

## 1 简介

TableGen 的目的是根据源文件中的信息生成复杂的输出文件。这些源文件比输出文件更易于编写，也更易于随时间推移进行维护和修改。信息以声明式风格编写，涉及类（class）和记录（record），随后由 TableGen 处理。内部化的记录被传递给各种后端，后端从记录子集中提取信息并生成一个或多个输出文件。这些输出文件通常是供 C++ 使用的 `.inc` 文件，但也可以是后端开发者所需的任何类型的文件。

本文档详细描述了 LLVM TableGen 设施，面向使用 TableGen 为项目生成代码的程序员。如果你只是寻求简单概览，请查阅 [TableGen 概述](https://llvm.org/docs/TableGen/index.html)。调用 TableGen 所使用的各种 `*-tblgen` 命令请参见 *tblgen Family - Description to C++ Code*。

后端的一个示例是 RegisterInfo，它为特定目标机器生成寄存器文件信息，供 LLVM 目标无关代码生成器使用。有关 LLVM TableGen 后端的描述，请参见 *TableGen Backends*；有关编写新后端的指南，请参见 *TableGen Backend Developer's Guide*。

后端能够完成的一些工作包括：

- 为特定目标机器生成寄存器文件信息。
- 为目标生成指令定义。
- 生成代码生成器用于将指令匹配到中间表示（IR）节点的模式。
- 为 Clang 生成语义属性标识符。
- 为 Clang 生成抽象语法树（AST）声明节点定义。
- 为 Clang 生成 AST 语句节点定义。

### 1.1 概念

TableGen 源文件包含两类主要条目：抽象记录和具体记录。在本文档及其他 TableGen 文档中，抽象记录称为**类**（class）。（这些类与 C++ 类不同，也不映射到 C++ 类。）此外，具体记录通常直接称为**记录**（record），尽管有时"record"一词也泛指类和具体记录，具体含义应由上下文判断。

类和具体记录都有唯一的名称，该名称由程序员选择或由 TableGen 生成。与名称关联的是一个带有值的字段列表，以及一个可选的父类列表（有时称为基类或超类）。字段是后端将处理的主要数据。需要注意的是，TableGen 不为字段赋予任何含义；含义完全由后端以及包含这些后端输出的程序来决定。

> **注意**  
> "父类"一词既可以指作为另一个类的父类，也可以指具体记录所继承的类。这种非标准用法是因为 TableGen 以相似的方式处理类和具体记录。

后端处理 TableGen 解析器构建的某个具体记录子集，并生成输出文件。这些文件通常是被需要这些记录数据的程序所包含的 C++ `.inc` 文件，但后端也可以生成任意类型的输出文件。例如，它可以生成包含带标识符和替换参数的消息的数据文件。在 LLVM 代码生成器这样复杂的用例中，可能存在许多具体记录，其中一些可能拥有出乎意料多的字段，从而产生大型输出文件。

为了降低 TableGen 文件的复杂性，类被用来抽象出一组记录字段。例如，几个类可以抽象机器寄存器文件的概念，其他类可以抽象指令格式，还有一些类可以抽象单条指令。TableGen 支持任意层次的类层级，因此可以为两个概念定义抽象类，并共享一个抽象了两个原始概念中公共"子概念"的第三个超类。

为了使类更有用，具体记录（或另一个类）可以将某个类作为父类，并向其传递模板参数。这些模板参数可以用于父类的字段中，以自定义方式初始化这些字段。也就是说，记录或类 A 可以用一组模板参数请求父类 S，而记录或类 B 可以用不同的参数请求 S。如果没有模板参数，将需要更多类，每种模板参数组合都需要一个类。

类和具体记录都可以包含未初始化的字段。未初始化的"值"用问号（`?`）表示。类通常包含未初始化的字段，期望在具体记录继承这些类时予以填充。即便如此，具体记录的某些字段也可能仍然未初始化。

TableGen 提供**多类**（multiclass）来将一组记录定义集中在一处。多类是一种可以被"调用"以一次定义多条具体记录的宏。多类可以继承自其他多类，这意味着该多类将继承父多类中的所有定义。

**附录 C：示例记录** 展示了 Intel X86 目标中的一条复杂记录，以及其简洁的定义方式。

---

## 2 源文件

TableGen 源文件是纯 ASCII 文本文件。文件可以包含语句、注释和空行（参见[词法分析](#3-词法分析)）。TableGen 文件的标准扩展名为 `.td`。

TableGen 文件可能会变得非常大，因此提供了包含机制，允许一个文件包含另一个文件的内容（参见[包含文件](#34-包含文件)）。这使得大文件可以分割成更小的文件，同时也提供了一种简单的库机制，允许多个源文件包含同一个库文件。

TableGen 支持一个简单的预处理器，可用于条件化 `.td` 文件的各部分。详情请参见[预处理设施](#9-预处理设施)。

---

## 3 词法分析

此处使用的词法和语法符号旨在模仿 Python 的符号。特别是对于词法定义，产生式在字符级别操作，元素之间不隐含空白。语法定义在词法单元级别操作，因此词法单元之间隐含空白。

TableGen 支持 BCPL 风格的注释（`// ...`）和可嵌套的 C 风格注释（`/* ... */`）。

TableGen 还提供简单的[预处理设施](#9-预处理设施)。

换页符可以在文件中自由使用，以便在打印文件时产生分页。

以下是基本标点符号词法单元：

```
- + [ ] { } ( ) < > : ; . ... = ? #
```

### 3.1 字面量

数值字面量采用以下格式之一：

```
TokInteger     ::=  DecimalInteger | HexInteger | BinInteger
DecimalInteger ::=  ["+" | "-"] ("0"..."9")+
HexInteger     ::=  "0x" ("0"..."9" | "a"..."f" | "A"..."F")+
BinInteger     ::=  "0b" ("0" | "1")+
```

注意，`DecimalInteger` 词法单元包含可选的 `+` 或 `-` 符号，这与大多数语言不同，因为在大多数语言中符号会被视为一元运算符。

TableGen 有两种字符串字面量：

```
TokString ::=  '"' (非'"'字符及转义序列) '"'
TokCode   ::=  "[{" (不含"}]"的文本) "}]"
```

`TokCode` 只是由 `[{` 和 `}]` 界定的多行字符串字面量。它可以跨行，换行符保留在字符串中。

当前实现支持以下转义序列：

```
\\ \' \" \t \n
```

### 3.2 标识符

TableGen 中的名称和标识符词法单元区分大小写。

```
ualpha        ::=  "a"..."z" | "A"..."Z" | "_"
TokIdentifier ::=  ("0"..."9")* ualpha (ualpha | "0"..."9")*
TokVarName    ::=  "$" ualpha (ualpha |  "0"..."9")*
```

注意，与大多数语言不同，TableGen 允许 `TokIdentifier` 以整数开头。在存在歧义时，词法单元被解释为数值字面量而非标识符。

TableGen 具有以下保留关键字，不能用作标识符：

```
assert     bit           bits          class         code
dag        def           dump          else          false
foreach    defm          defset        defvar        field
if         in            include       int           let
list       multiclass    string        then          true
```

> **警告**  
> 保留字 `field` 已废弃，除非在 `CodeEmitterGen` 后端中使用，在该后端中它用于区分普通记录字段和编码字段。

### 3.3 感叹号运算符

TableGen 提供"感叹号运算符"，具有多种用途：

```
BangOperator ::=  以下之一
                  !add         !and         !cast         !con         !dag
                  !div         !empty       !eq           !exists      !filter
                  !find        !foldl       !foreach      !ge          !getdagarg
                  !getdagname  !getdagop    !getdagopname !gt          !head
                  !if          !initialized !instances    !interleave  !isa
                  !le          !listconcat  !listflatten  !listremove  !listsplat
                  !logtwo      !lt          !match        !mul         !ne
                  !not         !or          !range        !repr        !setdagarg
                  !setdagname  !setdagop    !setdagopname !shl         !size
                  !sra         !srl         !strconcat    !sub         !subst
                  !substr      !tail        !tolower      !toupper     !xor
```

`!cond` 运算符的语法与其他感叹号运算符略有不同，因此单独定义：

```
CondOperator ::=  !cond
```

每个感叹号运算符的描述请参见[附录 A：感叹号运算符](#10-附录-a感叹号运算符)。

### 3.4 包含文件

TableGen 有包含机制。被包含文件的内容在词法上替换 `include` 指令，然后被解析，就好像它最初在主文件中一样。

```
IncludeDirective ::=  "include" TokString
```

主文件和被包含文件的各部分可以使用预处理器指令进行条件化：

```
PreprocessorDirective ::=  "#define" | "#ifdef" | "#ifndef"
```

---

## 4 类型

TableGen 语言是静态类型的，使用简单但完整的类型系统。类型用于检查错误、执行隐式转换，以及帮助接口设计者约束允许的输入。每个值都必须有关联类型。

TableGen 支持低级类型（如 `bit`）和高级类型（如 `dag`）的混合使用。这种灵活性允许你方便、紧凑地描述各种各样的记录。

```
Type    ::=  "bit" | "int" | "string" | "dag" | "code"
            | "bits" "<" TokInteger ">"
            | "list" "<" Type ">"
            | ClassID
ClassID ::=  TokIdentifier
```

**`bit`**  
`bit` 是布尔值，可以为 0 或 1。

**`int`**  
`int` 类型表示简单的 64 位整数值，如 `5` 或 `-42`。

**`string`**  
`string` 类型表示任意长度的有序字符序列。

**`code`**  
关键字 `code` 是 `string` 的别名，可用于表示代码的字符串值。

**`bits<n>`**  
`bits` 类型是任意长度 `n` 的定长整数，被视为独立的位。这些位可以单独访问。该类型的字段适合表示指令操作码、寄存器编号或地址模式/寄存器/偏移量。字段的位可以单独设置或作为子字段设置。例如，在指令地址中，寻址模式、基址寄存器编号和位移可以分别设置。

**`list<type>`**  
该类型表示元素类型为尖括号中指定类型的列表。元素类型是任意的，甚至可以是另一个列表类型。列表元素从 0 开始索引。

**`dag`**  
该类型表示节点的可嵌套有向无环图（DAG）。每个节点有一个运算符和零个或多个参数（或操作数）。参数可以是另一个 `dag` 对象，从而允许构建任意树状的节点和边。例如，DAG 被用于表示代码生成器指令选择算法所使用的代码模式。详情请参见[有向无环图（DAG）](#71-有向无环图dag)。

**`ClassID`**  
在类型上下文中指定类名，表示所定义值的类型必须是指定类的子类。这与 `list` 类型结合使用非常有用；例如，可以将列表的元素限制为公共基类（如 `list<Register>` 只能包含派生自 `Register` 类的定义）。`ClassID` 必须命名一个之前已声明或定义的类。

---

## 5 值与表达式

在 TableGen 语句的许多上下文中都需要值。一个常见的例子是在记录定义中，每个字段由名称和可选值指定。TableGen 在构建值表达式时允许相当多的不同形式，这些形式使得 TableGen 文件可以以对应用程序自然的语法编写。

注意，所有值都有从一种类型转换为另一种类型的规则。例如，这些规则允许你将像 `7` 这样的值赋给 `bits<4>` 类型的实体。

```
Value         ::=  SimpleValue ValueSuffix*
                  | Value "#" [Value]
ValueSuffix   ::=  RangeList
                  | "[" SliceElements "]"
                  | "." TokIdentifier
RangeList     ::=  "{" RangePiece ("," RangePiece)* "}"
RangePiece    ::=  TokInteger
                  | TokInteger "..." TokInteger
                  | TokInteger "-" TokInteger
                  | TokInteger TokInteger
SliceElements ::=  (SliceElement ",")* SliceElement ","?
SliceElement  ::=  Value
                  | Value "..." Value
                  | Value "-" Value
                  | Value TokInteger
```

> **警告**  
> `RangePiece` 和 `SliceElement` 的最后一种特殊形式是因为 `-` 被包含在 `TokInteger` 中，因此 `1-5` 被词法解析为两个连续词法单元，值为 `1` 和 `-5`，而不是 `"1"`、`"-"` 和 `"5"`。使用连字符作为范围标点已废弃。

### 5.1 简单值

`SimpleValue` 有多种形式：

```
SimpleValue  ::=  SimpleValue1
                 | SimpleValue2
                 | SimpleValue3
                 | SimpleValue4
                 | SimpleValue5
                 | SimpleValue6
                 | SimpleValue7
                 | SimpleValue8
                 | SimpleValue9
SimpleValue1 ::=  TokInteger | TokString+ | TokCode
```

值可以是整数字面量、字符串字面量或代码字面量。多个相邻的字符串字面量像 C/C++ 中一样被拼接；简单值是这些字符串的拼接。代码字面量变为字符串，此后与普通字符串无法区分。

```
SimpleValue2 ::=  "true" | "false"
```

`true` 和 `false` 字面量本质上是整数值 1 和 0 的语法糖。当布尔值用于字段初始化、位序列、`if` 语句等时，它们提高了 TableGen 文件的可读性。解析时，这些字面量被转换为整数。

> **注意**  
> 尽管 `true` 和 `false` 是 1 和 0 的字面量名称，但我们建议作为风格规范，仅将它们用于布尔值。

```
SimpleValue3 ::=  "?"
```

问号表示未初始化的值。

```
SimpleValue4 ::=  "{" [ValueList] "}"
ValueList    ::=  ValueListNE
ValueListNE  ::=  Value ("," Value)*
```

该值表示位序列，可用于初始化 `bits<n>` 字段（注意使用花括号）。这样做时，值必须合计表示 `n` 位。

```
SimpleValue5 ::=  "[" ValueList "]" ["<" Type ">"]
```

该值是列表初始化器（注意使用方括号）。方括号中的值是列表的元素。可选的 `Type` 可用于指示特定的元素类型；否则元素类型从给定值推断。TableGen 通常可以推断类型，但有时无法推断，例如值为空列表（`[]`）时。

```
SimpleValue6 ::=  "(" DagArg [DagArgList] ")"
DagArgList   ::=  DagArg ("," DagArg)*
DagArg       ::=  Value [":" TokVarName] | TokVarName
```

这表示 DAG 初始化器（注意使用圆括号）。第一个 `DagArg` 称为 DAG 的"运算符"，必须是一条记录。详情请参见[有向无环图（DAG）](#71-有向无环图dag)。

```
SimpleValue7 ::=  TokIdentifier
```

结果值是由标识符命名的实体的值。可能的标识符描述如下（阅读完本指南其余部分后，这些描述会更有意义）：

- 类的模板参数，例如在以下示例中对 `Bar` 的使用：
  ```
  class Foo <int Bar> {
    int Baz = Bar;
  }
  ```
- 类或多类定义中的隐式模板参数 `NAME`（参见 NAME 相关章节）。
- 类的局部字段，例如：
  ```
  class Foo {
    int Bar = 5;
    int Baz = Bar;
  }
  ```
- 记录定义的名称，例如在 `Foo` 的定义中对 `Bar` 的使用：
  ```
  def Bar : SomeClass {
    int X = 5;
  }
  def Foo {
    SomeClass Baz = Bar;
  }
  ```
- 记录定义的局部字段，例如：
  ```
  def Foo {
    int Bar = 5;
    int Baz = Bar;
  }
  ```
  从记录的父类继承的字段也可以用同样的方式访问。
- 多类的模板参数，例如：
  ```
  multiclass Foo <int Bar> {
    def : SomeClass<Bar>;
  }
  ```
- 用 `defvar` 或 `defset` 语句定义的变量。
- `foreach` 的迭代变量，例如：
  ```
  foreach i = 0...5 in
    def Foo#i;
  ```

```
SimpleValue8 ::=  ClassID "<" ArgValueList ">"
```

该形式创建一个新的匿名记录定义（就像由一个继承自给定类并带有给定模板参数的未命名 `def` 所创建的记录），其值就是该记录。可以使用后缀获取记录的字段；参见[带后缀的值](#52-带后缀的值)。以此方式调用类可以提供简单的子程序机制。详情请参见[将类用作子程序](#8-将类用作子程序)。

```
SimpleValue9 ::=  BangOperator ["<" Type ">"] "(" ValueListNE ")"
                 | CondOperator "(" CondClause ("," CondClause)* ")"
CondClause   ::=  Value ":" Value
```

感叹号运算符提供了其他简单值所不具备的函数。除 `!cond` 外，感叹号运算符接受一个括号括起来的参数列表，并对这些参数执行某些函数，为该感叹号运算符产生一个值。`!cond` 运算符接受一列由冒号分隔的参数对。详情请参见[附录 A：感叹号运算符](#10-附录-a感叹号运算符)。`Type` 仅对某些感叹号运算符接受，且不能为 `code`。

### 5.2 带后缀的值

上述 `SimpleValue` 值可以附加某些后缀。后缀的目的是获取主值的子值。以下是某些主值的可能后缀：

**`value{17}`**  
最终值是整数值的第 17 位（注意使用花括号）。

**`value{8...15}`**  
最终值是整数值的第 8-15 位。可以通过指定 `{15...8}` 来反转位的顺序。

**`value[i]`**  
最终值是列表值的第 `i` 个元素（注意使用方括号）。换句话说，方括号对列表起下标运算符的作用。这只在指定单个元素时适用。

**`value[i,]`**  
最终值是包含列表第 `i` 个单个元素的列表。简言之，包含单个元素的列表切片。

**`value[4...7,17,2...3,4]`**  
最终值是列表值的切片形成的新列表。新列表包含元素 4、5、6、7、17、2、3 和 4。元素可以多次包含，顺序任意。这只在指定多个元素时是结果。

**`value[i,m...n,j,ls]`**  
每个元素可以是表达式（变量、感叹号运算符）。`m` 和 `n` 的类型应为 `int`；`i`、`j` 和 `ls` 的类型应为 `int` 或 `list<int>`。

**`value.field`**  
最终值是指定记录值中指定字段的值。

### 5.3 粘贴运算符

粘贴运算符（`#`）是 TableGen 表达式中唯一可用的中缀运算符。它允许你拼接字符串或列表，但有一些不寻常的特性。

粘贴运算符可用于在 `Def` 或 `Defm` 语句中指定记录名称，此时必须构造一个字符串。如果操作数是未定义的名称（`TokIdentifier`）或全局 `Defvar` 或 `Defset` 的名称，则被视为逐字字符串。不使用全局名称的值。

粘贴运算符也可用于所有其他值表达式，此时它可以构造字符串或列表。有点奇怪但与前一情况一致的是，如果右侧操作数是未定义的名称或全局名称，则被视为逐字字符串。左侧操作数被正常处理。

值可以有尾随的粘贴运算符，此时左侧操作数被拼接到空字符串。

[附录 B：粘贴运算符示例](#11-附录-b粘贴运算符示例) 展示了粘贴运算符行为的示例。

---

## 6 语句

以下语句可以出现在 TableGen 源文件的顶层：

```
TableGenFile ::=  (Statement | IncludeDirective | PreprocessorDirective)*
Statement    ::=  Assert | Class | Def | Defm | Defset | Deftype
                 | Defvar | Dump  | Foreach | If | Let | MultiClass
```

### 6.1 `class` — 定义抽象记录类

`class` 语句定义一个抽象记录类，其他类和记录可以从中继承。

```
Class           ::=  "class" ClassID [TemplateArgList] RecordBody
TemplateArgList ::=  "<" TemplateArgDecl ("," TemplateArgDecl)* ">"
TemplateArgDecl ::=  Type TokIdentifier ["=" Value]
```

类可以通过"模板参数"列表参数化，其值可用于类的记录体。每次类被另一个类或记录继承时都需要指定这些模板参数。

如果模板参数没有用 `=` 赋默认值，则它是未初始化的（值为 `?`），并且在继承该类时必须在模板参数列表中指定（必需参数）。如果参数被赋了默认值，则在参数列表中可以不指定它（可选参数）。在声明中，所有必需的模板参数必须在任何可选参数之前。模板参数默认值从左到右求值。

`RecordBody` 定义见下文。它可以包含当前类所继承的父类列表，以及字段定义和其他语句。当类 C 继承自另一个类 D 时，D 的字段被有效地合并到 C 的字段中。

一个给定的类只能定义一次。满足以下任一条件时，`class` 语句被认为定义了该类（`RecordBody` 的元素在下面描述）：

- 存在 `TemplateArgList`，或
- `RecordBody` 中存在 `ParentClassList`，或
- `RecordBody` 中的 `Body` 存在且非空。

你可以通过指定空的 `TemplateArgList` 和空的 `RecordBody` 来声明一个空类。这可以作为一种受限的前向声明形式。注意，从前向声明的类派生的记录不会从中继承任何字段，因为这些记录在其声明被解析时构建，因此在类最终定义之前。

每个类都有一个名为 `NAME`（大写）的隐式模板参数，它被绑定到继承自该类的 `Def` 或 `Defm` 的名称。如果类被匿名记录继承，则名称未指定但全局唯一。

示例请参见[示例：类与记录](#63-示例类与记录)。

#### 6.1.1 记录体

记录体出现在类和记录定义中。记录体可以包含父类列表，指定当前类或记录从哪些类继承字段。此类类称为类或记录的**父类**。记录体还包括定义的主体，其中包含类或记录的字段规范。

```
RecordBody            ::=  ParentClassList Body
ParentClassList       ::=  [":" ParentClassListNE]
ParentClassListNE     ::=  ClassRef ("," ClassRef)*
ClassRef              ::=  (ClassID | MultiClassID) ["<" [ArgValueList] ">"]
ArgValueList          ::=  PostionalArgValueList [","] NamedArgValueList
PostionalArgValueList ::=  [Value {"," Value}*]
NamedArgValueList     ::=  [NameValue "=" Value {"," NameValue "=" Value}*]
```

包含 `MultiClassID` 的 `ParentClassList` 仅在 `defm` 语句的类列表中有效。在这种情况下，ID 必须是多类的名称。

参数值可以以两种形式指定：

- **位置参数**（`value`）：值被赋给对应位置的参数。对于 `Foo<a0, a1>`，`a0` 将被赋给第一个参数，`a1` 将被赋给第二个参数。
- **命名参数**（`name=value`）：值被赋给具有指定名称的参数。对于 `Foo<a=a0, b=a1>`，`a0` 将被赋给名为 `a` 的参数，`a1` 将被赋给名为 `b` 的参数。

必需参数也可以指定为命名参数。注意，无论以何种方式（命名或位置）指定，参数只能指定一次，且位置参数必须在命名参数之前。

```
Body     ::=  ";" | "{" BodyItem* "}"
BodyItem ::=  Type TokIdentifier ["=" Value] ";"
             | "let" [LetMode] TokIdentifier [RangeList] "=" Value ";"
             | "defvar" TokIdentifier "=" Value ";"
             | Assert
LetMode  ::=  "append" | "prepend"
```

注意，`append` 和 `prepend` 是上下文敏感关键字：它们只在 `let` 之后立即作为修饰符被识别。在所有其他位置，它们仍然是有效的标识符（例如可用作字段名）。

体中的字段定义指定要包含在类或记录中的字段。如果没有指定初始值，则字段的值是未初始化的。必须指定类型；TableGen 不会从值推断类型。

`let` 形式用于将字段重置为新值。这可以针对直接在体中定义的字段或从父类继承的字段进行。可以指定 `RangeList` 以重置 `bit<n>` 字段中的某些位。

`let append` 和 `let prepend` 形式将值与字段的当前值拼接，而不是替换它。对于 `append`，新值添加在当前值之后；对于 `prepend`，新值添加在当前值之前。支持的类型和拼接运算符为：

- `list<T>`：使用 `!listconcat`
- `string` / `code`：使用 `!strconcat`
- `dag`：使用 `!con`

如果字段当前未设置（`?`），`let append` 和 `let prepend` 会直接设置值。这对于在类层次结构中累积值很有用：

```tablegen
class Base {
  list<int> items = [2, 3];
}
class Middle : Base {
  let append items = [4];       // items = [2, 3, 4]
}
def Concrete : Middle {
  let prepend items = [1];      // items = [1, 2, 3, 4]
}
```

普通的 `let`（不带 `append`/`prepend`）总是替换当前值，可用于退出累积值。

`defvar` 形式定义一个变量，其值可在体内的其他值表达式中使用。该变量不是字段：它不会成为正在定义的类或记录的字段。变量用于在处理体时保存临时值。详情请参见[记录体中的 defvar](#72-记录体中的-defvar)。

当类 C2 继承自类 C1 时，它获得 C1 的所有字段定义。当这些定义被合并到类 C2 中时，C2 传递给 C1 的任何模板参数都会被替换到这些定义中。换句话说，C1 定义的抽象记录字段在合并到 C2 之前会用模板参数展开。

### 6.2 `def` — 定义具体记录

`def` 语句定义一条新的具体记录。

```
Def       ::=  "def" [NameValue] RecordBody
NameValue ::=  Value （以特殊模式解析）
```

名称值是可选的。如果指定，它以一种特殊模式解析，其中未定义（未识别）的标识符被解释为字面字符串。特别是，全局标识符被视为未识别。这包括由 `defvar` 和 `defset` 定义的全局变量。记录名称可以是空字符串。

如果没有给出名称值，则记录是匿名的。匿名记录的最终名称未指定但全局唯一。

如果 `def` 出现在 `multiclass` 语句中，会发生特殊处理。详情请参见下面的 `multiclass` 部分。

记录可以通过在其记录体开头指定 `ParentClassList` 子句从一个或多个类继承。父类中的所有字段都被添加到记录中。如果两个或多个父类提供了相同的字段，则记录最终具有最后一个父类的字段值。

作为特殊情况，记录的名称可以作为模板参数传递给该记录的父类。例如：

```tablegen
class A <dag d> {
  dag the_dag = d;
}

def rec1 : A<(ops rec1)>;
```

DAG `(ops rec1)` 作为模板参数传递给类 A。注意 DAG 包含正在定义的记录 `rec1`。

创建新记录所采取的步骤相当复杂，请参见[记录的构建过程](#73-记录的构建过程)。

示例请参见[示例：类与记录](#63-示例类与记录)。

### 6.3 示例：类与记录

这是一个包含一个类和两个记录定义的简单 TableGen 文件：

```tablegen
class C {
  bit V = true;
}

def X : C;
def Y : C {
  let V = false;
  string Greeting = "Hello!";
}
```

首先定义抽象类 C，它有一个名为 `V` 的字段，类型为 `bit`，初始化为 `true`。

接下来定义了两条记录，都派生自类 C（即以 C 为父类）。因此它们都继承了 `V` 字段。记录 Y 还定义了另一个字符串字段 `Greeting`，初始化为 `"Hello!"`。此外，Y 覆盖了继承的 `V` 字段，将其设置为 `false`。

类对于将多个记录的公共特性隔离在一处非常有用。类可以将公共字段初始化为默认值，但继承自该类的记录可以覆盖这些默认值。

TableGen 支持参数化类和非参数化类的定义。参数化类指定变量声明列表，这些变量可以有可选的默认值，在类被指定为另一个类或记录的父类时绑定。

```tablegen
class FPFormat <bits<3> val> {
  bits<3> Value = val;
}

def NotFP      : FPFormat<0>;
def ZeroArgFP  : FPFormat<1>;
def OneArgFP   : FPFormat<2>;
def OneArgFPRW : FPFormat<3>;
def TwoArgFP   : FPFormat<4>;
def CompareFP  : FPFormat<5>;
def CondMovFP  : FPFormat<6>;
def SpecialFP  : FPFormat<7>;
```

`FPFormat` 类的目的是充当一种枚举类型。它提供一个字段 `Value`，保存一个 3 位数字。其模板参数 `val` 用于设置 `Value` 字段。八条记录都以 `FPFormat` 为父类定义，枚举值在尖括号中作为模板参数传递。每条记录将继承带有适当枚举值的 `Value` 字段。

这里有一个带模板参数的类的更复杂示例，展示了如何定义类来重新组织另一个类的字段，从而隐藏该类的内部表示：

```tablegen
class ModRefVal <bits<2> val> {
  bits<2> Value = val;
}

def None   : ModRefVal<0>;
def Mod    : ModRefVal<1>;
def Ref    : ModRefVal<2>;
def ModRef : ModRefVal<3>;

class ModRefBits <ModRefVal mrv> {
  bit isMod = mrv.Value{0};
  bit isRef = mrv.Value{1};
}

def foo   : ModRefBits<Mod>;
def bar   : ModRefBits<Ref>;
def snork : ModRefBits<ModRef>;
```

对此示例运行 `llvm-tblgen` 会打印以下定义：

```
def bar {      // Value
  bit isMod = 0;
  bit isRef = 1;
}
def foo {      // Value
  bit isMod = 1;
  bit isRef = 0;
}
def snork {      // Value
  bit isMod = 1;
  bit isRef = 1;
}
```

### 6.4 `let` — 覆盖类或记录中的字段

`let` 语句收集一组字段值（有时称为绑定），并将它们应用于该 `let` 范围内的语句所定义的所有类和记录。

```
Let     ::=   "let" LetList "in" "{" Statement* "}"
            | "let" LetList "in" Statement
LetList ::=  LetItem ("," LetItem)*
LetItem ::=  [LetMode] TokIdentifier [RangeList] "=" Value
```

`let` 语句建立一个作用域，即花括号中的语句序列或不带花括号的单条语句。`LetList` 中的绑定应用于该作用域中的语句。

`LetList` 中的字段名必须命名由语句定义的类和记录所继承的类中的字段。字段值在记录从父类继承所有字段之后应用于类和记录。因此 `let` 的作用是覆盖继承的字段值。`let` 不能覆盖模板参数的值。

当少数字段需要在多条记录中被覆盖时，顶层 `let` 语句非常有用。以下是两个示例，注意 `let` 语句可以嵌套：

```tablegen
let isTerminator = true, isReturn = true, isBarrier = true, hasCtrlDep = true in
  def RET : I<0xC3, RawFrm, (outs), (ins), "ret", [(X86retflag 0)]>;

let isCall = true in
  let Defs = [EAX, ECX, EDX, FP0, FP1, FP2, FP3, FP4, FP5, FP6, ST0,
              MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7, XMM0, XMM1, XMM2,
              XMM3, XMM4, XMM5, XMM6, XMM7, EFLAGS] in {
    def CALLpcrel32 : Ii32<0xE8, RawFrm, (outs), (ins i32imm:$dst, variable_ops),
                           "call\t${dst:call}", []>;
    def CALL32r     : I<0xFF, MRM2r, (outs), (ins GR32:$dst, variable_ops),
                        "call\t{*}$dst", [(X86call GR32:$dst)]>;
    def CALL32m     : I<0xFF, MRM2m, (outs), (ins i32mem:$dst, variable_ops),
                        "call\t{*}$dst", []>;
  }
```

注意顶层 `let` 不会覆盖类或记录本身中定义的字段。

顶层 `let` 也支持 `append` 和 `prepend` 模式，将值与字段的当前值拼接而不是替换：

```tablegen
let append traits = [NewTrait] in {
  def MyRecord : Base;
}
```

### 6.5 `multiclass` — 定义多条记录

虽然带模板参数的类是在多条记录之间提取共性的好方法，但多类提供了一种一次定义多条记录的便利方法。例如，考虑一种 3 地址指令架构，其指令有两种格式：`reg = reg op reg` 和 `reg = reg op imm`（如 SPARC）。我们希望在一处指定这两种常见格式存在，然后在另一处指定所有操作。`multiclass` 和 `defm` 语句实现了这一目标。你可以把多类看作是展开为多条记录的宏或模板。

```
MultiClass          ::=  "multiclass" TokIdentifier [TemplateArgList]
                         ParentClassList
                         "{" MultiClassStatement+ "}"
MultiClassID        ::=  TokIdentifier
MultiClassStatement ::=  Assert | Def | Defm | Defvar | Foreach | If | Let
```

与普通类一样，多类有名称并可以接受模板参数。多类可以继承自其他多类，这导致其他多类被展开并为继承的多类中的记录定义做出贡献。多类的体包含一系列使用 `Def` 和 `Defm` 定义记录的语句。此外，`Defvar`、`Foreach` 和 `Let` 语句可用于提取更多公共元素。`If` 和 `Assert` 语句也可以使用。

与普通类一样，多类也有隐式模板参数 `NAME`（参见 NAME 相关章节）。当在多类中定义一个命名（非匿名）记录，且记录名称不包含对模板参数 `NAME` 的使用时，这样的使用会自动前置到名称中。也就是说，在多类中以下两种写法等价：

```tablegen
def Foo ...
def NAME # Foo ...
```

多类中定义的记录在多类通过 `defm` 语句在多类定义之外被"实例化"或"调用"时创建。多类中的每个 `def` 语句都会产生一条记录。与顶层 `def` 语句一样，这些定义可以继承自多个父类。

示例请参见[示例：多类与 defm](#67-示例多类与-defm)。

### 6.6 `defm` — 调用多类以定义多条记录

一旦定义了多类，你就可以使用 `defm` 语句来"调用"它们并处理那些多类中的多个记录定义。这些记录定义由多类中的 `def` 语句指定，间接地也由 `defm` 语句指定。

```
Defm ::=  "defm" [NameValue] ParentClassList ";"
```

可选的 `NameValue` 的形成方式与 `def` 的名称相同。`ParentClassList` 是冒号后跟至少一个多类和任意数量的普通类的列表。多类必须在普通类之前。注意 `defm` 没有体。

该语句实例化所有指定多类中定义的所有记录，无论是直接通过 `def` 语句还是间接通过 `defm` 语句。这些记录也接收父类列表中包含的任何普通类中定义的字段。这对于向 `defm` 创建的所有记录添加一组公共字段很有用。

名称以 `def` 使用的相同特殊模式解析。如果未包含名称，则提供一个未指定但全局唯一的名称。也就是说，以下示例最终具有不同的名称：

```tablegen
defm    : SomeMultiClass<...>;   // 全局唯一名称
defm "" : SomeMultiClass<...>;   // 空名称
```

`defm` 语句可以在多类体中使用。在这种情况下，第二种变体等价于：

```tablegen
defm NAME : SomeMultiClass<...>;
```

更一般地，当 `defm` 出现在多类中且其名称不包含对隐式模板参数 `NAME` 的使用时，`NAME` 将被自动前置。也就是说，在多类中以下两种写法等价：

```tablegen
defm Foo        : SomeMultiClass<...>;
defm NAME # Foo : SomeMultiClass<...>;
```

示例请参见[示例：多类与 defm](#67-示例多类与-defm)。

### 6.7 示例：多类与 defm

这是一个使用 `multiclass` 和 `defm` 的简单示例。考虑一种 3 地址指令架构，其指令有两种格式：`reg = reg op reg` 和 `reg = reg op imm`（立即数）。SPARC 就是这样的架构。

```tablegen
def ops;
def GPR;
def Imm;
class inst <int opc, string asmstr, dag operandlist>;

multiclass ri_inst <int opc, string asmstr> {
  def _rr : inst<opc, !strconcat(asmstr, " $dst, $src1, $src2"),
                   (ops GPR:$dst, GPR:$src1, GPR:$src2)>;
  def _ri : inst<opc, !strconcat(asmstr, " $dst, $src1, $src2"),
                   (ops GPR:$dst, GPR:$src1, Imm:$src2)>;
}

// 为 RR 和 RI 格式中的每条指令定义记录
defm ADD : ri_inst<0b111, "add">;
defm SUB : ri_inst<0b101, "sub">;
defm MUL : ri_inst<0b100, "mul">;
```

每次使用 `ri_inst` 多类都定义两条记录，一条带 `_rr` 后缀，一条带 `_ri`。回顾一下，使用多类的 `defm` 的名称会前置到该多类中定义的记录名称中。因此最终的定义名称为：

```
ADD_rr, ADD_ri
SUB_rr, SUB_ri
MUL_rr, MUL_ri
```

没有多类特性时，指令必须如下定义：

```tablegen
def ops;
def GPR;
def Imm;
class inst <int opc, string asmstr, dag operandlist>;

class rrinst <int opc, string asmstr>
  : inst<opc, !strconcat(asmstr, " $dst, $src1, $src2"),
           (ops GPR:$dst, GPR:$src1, GPR:$src2)>;

class riinst <int opc, string asmstr>
  : inst<opc, !strconcat(asmstr, " $dst, $src1, $src2"),
           (ops GPR:$dst, GPR:$src1, Imm:$src2)>;

def ADD_rr : rrinst<0b111, "add">;
def ADD_ri : riinst<0b111, "add">;
def SUB_rr : rrinst<0b101, "sub">;
def SUB_ri : riinst<0b101, "sub">;
def MUL_rr : rrinst<0b100, "mul">;
def MUL_ri : riinst<0b100, "mul">;
```

`defm` 还可以在多类中使用，通过 `defm` 调用其他多类并在这些多类中创建记录：

```tablegen
class Instruction <bits<4> opc, string Name> {
  bits<4> opcode = opc;
  string name = Name;
}

multiclass basic_r <bits<4> opc> {
  def rr : Instruction<opc, "rr">;
  def rm : Instruction<opc, "rm">;
}

multiclass basic_s <bits<4> opc> {
  defm SS : basic_r<opc>;
  defm SD : basic_r<opc>;
  def X : Instruction<opc, "x">;
}

multiclass basic_p <bits<4> opc> {
  defm PS : basic_r<opc>;
  defm PD : basic_r<opc>;
  def Y : Instruction<opc, "y">;
}

defm ADD : basic_s<0xf>, basic_p<0xf>;
```

最终的 `defm` 创建了以下记录，5 条来自 `basic_s` 多类，5 条来自 `basic_p` 多类：

```
ADDSSrr, ADDSSrm
ADDSDrr, ADDSDrm
ADDX
ADDPSrr, ADDPSrm
ADDPDrr, ADDPDrm
ADDY
```

`defm` 语句（无论是顶层还是在多类中）都可以继承普通类以及多类。规则是普通类必须列在多类之后，并且必须至少有一个多类：

```tablegen
class XD { bits<4> Prefix = 11; }
class XS { bits<4> Prefix = 12; }
class I <bits<4> op> { bits<4> opcode = op; }

multiclass R {
  def rr : I<4>;
  def rm : I<2>;
}

multiclass Y {
  defm SS : R, XD;    // 先是多类 R，然后是普通类 XD
  defm SD : R, XS;
}

defm Instr : Y;
```

此示例将创建四条记录（按字母顺序排列）：

```
def InstrSDrm { bits<4> opcode = { 0, 0, 1, 0 }; bits<4> Prefix = { 1, 1, 0, 0 }; }
def InstrSDrr { bits<4> opcode = { 0, 1, 0, 0 }; bits<4> Prefix = { 1, 1, 0, 0 }; }
def InstrSSrm { bits<4> opcode = { 0, 0, 1, 0 }; bits<4> Prefix = { 1, 0, 1, 1 }; }
def InstrSSrr { bits<4> opcode = { 0, 1, 0, 0 }; bits<4> Prefix = { 1, 0, 1, 1 }; }
```

在多类中使用 `let` 语句也是可能的，这提供了另一种提取公共元素的方式：

```tablegen
multiclass basic_r <bits<4> opc> {
  let Predicates = [HasSSE2] in {
    def rr : Instruction<opc, "rr">;
    def rm : Instruction<opc, "rm">;
  }
  let Predicates = [HasSSE3] in
    def rx : Instruction<opc, "rx">;
}

multiclass basic_ss <bits<4> opc> {
  let IsDouble = false in
    defm SS : basic_r<opc>;

  let IsDouble = true in
    defm SD : basic_r<opc>;
}

defm ADD : basic_ss<0xf>;
```

### 6.8 `defset` — 创建定义集合

`defset` 语句用于将一组记录收集到全局记录列表中。

```
Defset ::=  "defset" Type TokIdentifier "=" "{" Statement* "}"
```

花括号内通过 `def` 和 `defm` 定义的所有记录像往常一样定义，同时也被收集到给定名称（`TokIdentifier`）的全局列表中。

指定的类型必须是 `list<class>`，其中 `class` 是某个记录类。`defset` 语句为其语句建立作用域。在 `defset` 的作用域内定义不属于 `class` 类型的记录是错误的。

`defset` 语句可以嵌套。内部 `defset` 将记录添加到自己的集合中，所有这些记录也被添加到外部集合中。

使用 `ClassID<...>` 语法在初始化表达式内创建的匿名记录不会被收集到集合中。

### 6.9 `deftype` — 定义类型

`deftype` 语句定义一个类型，该类型可在定义之后的语句中使用。

```
Deftype ::=  "deftype" TokIdentifier "=" Type ";"
```

`=` 左边的标识符被定义为类型名称，其实际类型由 `=` 右边的类型表达式给出。目前，只支持原始类型和类型别名作为源类型，且 `deftype` 语句只能出现在顶层。

### 6.10 `defvar` — 定义变量

`defvar` 语句定义一个全局变量，其值可在定义之后的语句中使用。

```
Defvar ::=  "defvar" TokIdentifier "=" Value ";"
```

`=` 左边的标识符被定义为全局变量，其值由 `=` 右边的值表达式给出。变量的类型被自动推断。

一旦定义了变量，就不能将其设置为另一个值。在顶层 `foreach` 中定义的变量在每次循环迭代结束时超出作用域，因此其在一次迭代中的值在下一次迭代中不可用。以下 `defvar` 将不起作用：

```tablegen
defvar i = !add(i, 1);
```

变量也可以用 `defvar` 在记录体中定义，详情请参见[记录体中的 defvar](#72-记录体中的-defvar)。

### 6.11 `foreach` — 遍历语句序列

`foreach` 语句遍历一系列语句，使变量在一个值序列上变化。

```
Foreach         ::=  "foreach" ForeachIterator "in" "{" Statement* "}"
                    | "foreach" ForeachIterator "in" Statement
ForeachIterator ::=  TokIdentifier "=" (RangeList | RangePiece | Value)
```

`foreach` 的体是花括号中的语句序列或不带花括号的单条语句。对于范围列表、范围片段或单个值中的每个值，语句都会被重新求值。在每次迭代中，`TokIdentifier` 变量被设置为该值，可以在语句中使用。

语句列表建立一个内部作用域。`foreach` 的局部变量在每次循环迭代结束时超出作用域，因此其值不会从一次迭代延续到下一次。`foreach` 循环可以嵌套。

```tablegen
foreach i = [0, 1, 2, 3] in {
  def R#i : Register<...>;
  def F#i : Register<...>;
}
```

此循环定义名为 `R0`、`R1`、`R2` 和 `R3` 的记录，以及 `F0`、`F1`、`F2` 和 `F3`。

### 6.12 `dump` — 向标准错误输出打印消息

`dump` 语句将输入字符串打印到标准错误输出。它用于调试目的。

- 在顶层，消息立即打印。
- 在记录/类/多类内，`dump` 在包含记录的每个实例化点被求值。

```
Dump ::=  "dump" Value ";"
```

`Value` 是任意字符串表达式。例如，它可以与 `!repr` 结合使用来检查传递给多类的值：

```tablegen
multiclass MC<dag s> {
  dump "s = " # !repr(s);
}
```

### 6.13 `if` — 基于测试选择语句

`if` 语句允许基于表达式的值选择两组语句之一。

```
If     ::=  "if" Value "then" IfBody
           | "if" Value "then" IfBody "else" IfBody
IfBody ::=  "{" Statement* "}" | Statement
```

值表达式被求值。如果求值为真（与感叹号运算符使用的相同意义），则处理 `then` 保留字后面的语句。否则，如果存在 `else` 保留字，则处理 `else` 后面的语句。如果值为假且没有 `else` 分支，则不处理任何语句。

由于 `then` 语句周围的花括号是可选的，此语法规则具有"悬挂 else"子句的常见歧义性，以通常的方式解决：在 `if v1 then if v2 then {...} else {...}` 这样的情况下，`else` 与内层 `if` 关联而不是外层 `if`。

`then` 和 `else` 分支的 `IfBody` 建立内部作用域。体中定义的任何 `defvar` 变量在体结束时超出作用域（详情请参见[记录体中的 defvar](#72-记录体中的-defvar)）。

`if` 语句也可以在记录 `Body` 中使用。

### 6.14 `assert` — 检查条件是否为真

`assert` 语句检查布尔条件是否为真，如果不为真则打印错误消息。

```
Assert ::=  "assert" Value "," Value ";"
```

第一个 `Value` 是布尔条件。如果为真，语句不做任何事。如果条件为假，则打印非致命错误消息。第二个 `Value` 是消息，可以是任意字符串表达式，作为注释包含在错误消息中。`assert` 语句的确切行为取决于其放置位置：

- **在顶层**：立即检查断言。
- **在记录定义中**：语句被保存，所有断言在记录完全构建后检查。
- **在类定义中**：断言被保存，并由继承自该类的所有子类和记录继承。断言在记录完全构建时检查。
- **在多类定义中**：断言与多类的其他组件一起保存，然后在每次用 `defm` 实例化多类时检查。

在 TableGen 文件中使用断言可以简化 TableGen 后端中的记录检查。以下是两个类定义中的 `assert` 示例：

```tablegen
class PersonName<string name> {
  assert !le(!size(name), 32), "person name is too long: " # name;
  string Name = name;
}

class Person<string name, int age> : PersonName<name> {
  assert !and(!ge(age, 1), !le(age, 120)), "person age is invalid: " # age;
  int Age = age;
}

def Rec20 : Person<"Donald Knuth", 60> {
  ...
}
```

---

## 7 附加细节

### 7.1 有向无环图（DAG）

有向无环图可以直接在 TableGen 中使用 `dag` 数据类型表示。DAG 节点由一个运算符和零个或多个参数（或操作数）组成。每个参数可以是任意所需类型。通过使用另一个 DAG 节点作为参数，可以构建任意 DAG 节点图。

`dag` 实例的语法为：

```
( operator argument1, argument2, … )
```

运算符必须存在，且必须是一条记录。可以有零个或多个参数，用逗号分隔。运算符和参数可以有三种格式：

| 格式 | 含义 |
|------|------|
| `value` | 参数值 |
| `value:name` | 参数值及其关联名称 |
| `name` | 参数名称，值未设置（未初始化） |

值可以是任何 TableGen 值。如果存在名称，必须是以美元符号（`$`）开头的 `TokVarName`。名称的目的是用特定含义标记 DAG 中的运算符或参数，或将一个 DAG 中的参数与另一个 DAG 中同名参数关联起来。

以下感叹号运算符在处理 DAG 时很有用：`!con`、`!dag`、`!empty`、`!foreach`、`!getdagarg`、`!getdagname`、`!getdagop`、`!getdagopname`、`!setdagarg`、`!setdagname`、`!setdagop`、`!setdagopname`、`!size`。

### 7.2 记录体中的 defvar

除了定义全局变量外，`defvar` 语句还可以在类或记录定义的 `Body` 中用于定义局部变量。类或多类的模板参数可以在值表达式中使用。变量的作用域从 `defvar` 语句延伸到体的末尾。在其作用域内不能设置为不同的值。`defvar` 语句也可以在 `foreach` 的语句列表中使用，这建立了一个作用域。

内部作用域中的变量 V 会遮蔽（隐藏）外部作用域中的任何变量 V。特别是有几种情况：

- 记录体中的 V 遮蔽全局 V。
- 记录体中的 V 遮蔽模板参数 V。
- 模板参数中的 V 遮蔽全局 V。
- `foreach` 语句列表中的 V 遮蔽周围记录或全局作用域中的任何 V。

在 `foreach` 中定义的变量在每次循环迭代结束时超出作用域，因此其在一次迭代中的值在下一次迭代中不可用。以下 `defvar` 将不起作用：

```tablegen
defvar i = !add(i, 1)
```

### 7.3 记录的构建过程

TableGen 在构建记录时采取以下步骤。类是简单的抽象记录，因此经历相同的步骤。

1. 构建记录名称（`NameValue`）并创建空记录。
2. 从左到右解析 `ParentClassList` 中的父类，从上到下访问每个父类的祖先类：
   - 将父类的字段添加到记录中。
   - 将模板参数替换到这些字段中。
   - 将父类添加到记录的继承类列表中。
3. 将任何顶层 `let` 绑定应用于记录。回想一下，顶层绑定只应用于继承的字段。
4. 解析记录的体：
   - 向记录添加任何字段。
   - 根据局部 `let` 语句修改字段的值。
   - 定义任何 `defvar` 变量。
5. 对所有字段进行一次遍历以解析任何字段间引用。
6. 将记录添加到最终记录列表中。

由于字段间引用在步骤 5 中解析（在步骤 3 应用 `let` 绑定之后），`let` 语句具有不寻常的能力。例如：

```tablegen
class C <int x> {
  int Y = x;
  int Yplus1 = !add(Y, 1);
  int xplus1 = !add(x, 1);
}

let Y = 10 in {
  def rec1 : C<5> {}
}

def rec2 : C<5> {
  let Y = 10;
}
```

在两种情况下（使用顶层 `let` 绑定 Y 和使用局部 `let` 做同样的事），结果都是：

```
def rec1 {      // C
  int Y = 10;
  int Yplus1 = 11;
  int xplus1 = 6;
}
def rec2 {      // C
  int Y = 10;
  int Yplus1 = 11;
  int xplus1 = 6;
}
```

`Yplus1` 为 11，因为 `let Y` 在 `!add(Y, 1)` 被解析之前执行。请谨慎使用这种能力。

---

## 8 将类用作子程序

如[简单值](#51-简单值)中所述，类可以在表达式中被调用并传递模板参数。这使得 TableGen 创建一个继承自该类的新匿名记录。像往常一样，记录接收类中定义的所有字段。

该特性可以用作简单的子程序机制。类可以使用模板参数定义各种变量和字段，这些变量和字段最终出现在匿名记录中。然后可以在调用类的表达式中检索这些字段，如下所示。假设字段 `ret` 包含子程序的最终值：

```tablegen
int Result = ... CalcValue<arg>.ret ...;
```

`CalcValue` 类以模板参数 `arg` 调用。它为 `ret` 字段计算一个值，然后在 `Result` 字段初始化的"调用点"检索该值。此示例中创建的匿名记录除了携带结果值外没有其他用途。

以下是一个实际示例。类 `isValidSize` 确定指定的字节数是否表示有效的数据大小。`bit ret` 被适当地设置。字段 `ValidSize` 通过以数据大小调用 `isValidSize` 并从结果匿名记录中检索 `ret` 字段来获得其初始值：

```tablegen
class isValidSize<int size> {
  bit ret = !cond(!eq(size,  1): 1,
                  !eq(size,  2): 1,
                  !eq(size,  4): 1,
                  !eq(size,  8): 1,
                  !eq(size, 16): 1,
                  true: 0);
}

def Data1 {
  int Size = ...;
  bit ValidSize = isValidSize<Size>.ret;
}
```

---

## 9 预处理设施

嵌入在 TableGen 中的预处理器仅用于简单的条件编译。它支持以下指令（以某种非正式方式指定）：

```
LineBegin              ::=  行首
LineEnd                ::=  换行 | 回车 | EOF
WhiteSpace             ::=  空格 | 制表符
CComment               ::=  "/*" ... "*/"
BCPLComment            ::=  "//" ... LineEnd
WhiteSpaceOrCComment   ::=  WhiteSpace | CComment
WhiteSpaceOrAnyComment ::=  WhiteSpace | CComment | BCPLComment
MacroName              ::=  ualpha (ualpha | "0"..."9")*
PreDefine              ::=  LineBegin (WhiteSpaceOrCComment)*
                            "#define" (WhiteSpace)+ MacroName
                            (WhiteSpaceOrAnyComment)* LineEnd
PreIfdef               ::=  LineBegin (WhiteSpaceOrCComment)*
                            ("#ifdef" | "#ifndef") (WhiteSpace)+ MacroName
                            (WhiteSpaceOrAnyComment)* LineEnd
PreElse                ::=  LineBegin (WhiteSpaceOrCComment)*
                            "#else" (WhiteSpaceOrAnyComment)* LineEnd
PreEndif               ::=  LineBegin (WhiteSpaceOrCComment)*
                            "#endif" (WhiteSpaceOrAnyComment)* LineEnd
```

`MacroName` 可以在 TableGen 文件中的任意位置定义。该名称没有值；只能测试它是否已定义。

宏测试区域以 `#ifdef` 或 `#ifndef` 指令开始。如果宏名称已定义（`#ifdef`）或未定义（`#ifndef`），则处理指令和对应的 `#else` 或 `#endif` 之间的源代码。如果测试失败但有 `#else` 子句，则处理 `#else` 和 `#endif` 之间的源代码。如果测试失败且没有 `#else` 子句，则不处理测试区域中的任何源代码。

测试区域可以嵌套，但必须正确嵌套。在文件中开始的区域必须在该文件中结束；即 `#endif` 必须在同一文件中。

`MacroName` 可以使用 `*-tblgen` 命令行上的 `-D` 选项从外部定义：

```bash
llvm-tblgen self-reference.td -Dmacro1 -Dmacro3
```

---

## 10 附录 A：感叹号运算符

感叹号运算符在值表达式中充当函数。感叹号运算符接受一个或多个参数，对其进行操作，并产生结果。如果运算符产生布尔结果，结果值为 1 表示真，0 表示假。当运算符测试布尔参数时，0 被解释为假，非 0 被解释为真。

---

**`!add(a, b, ...)`**  
该运算符将 a、b 等相加，产生和。

---

**`!and(a, b, ...)`**  
该运算符对 a、b 等进行按位 AND，产生结果。如果所有参数都是 0 或 1，则可以执行逻辑 AND。当最左边的操作数为 0 时，此运算符短路为 0。

---

**`!cast<type>(a)`**  
该运算符对 a 执行强制类型转换并产生结果。如果 a 不是字符串，则执行直接类型转换，例如 `int` 和 `bit` 之间，或记录类型之间。这允许将记录强制转换为类。如果将记录强制转换为字符串，则产生记录的名称。

如果 a 是字符串，则将其视为记录名称，并在所有已定义记录的列表中查找。生成的记录应为指定类型。

例如，如果 `!cast<type>(name)` 出现在多类定义中，或出现在多类定义内实例化的类中，且 name 不引用多类的任何模板参数，则具有该名称的记录必须已在源文件中更早地实例化。如果 name 确实引用了模板参数，则查找推迟到实例化多类的 `defm` 语句（或更晚，如果 `defm` 出现在另一个多类中，且引用 name 的内层多类的模板参数被包含对外层多类模板参数的引用的值替换）。

如果 a 的类型与 type 不匹配，TableGen 会引发错误。

---

**`!con(a, b, ...)`**  
该运算符拼接 DAG 节点 a、b 等。它们的运算符必须相等。  
`!con((op:$lhs a1:$name1, a2:$name2), (op:$rhs b1:$name3))` 结果为 DAG 节点 `(op:$lhs a1:$name1, a2:$name2, b1:$name3)`。dag 运算符的名称从 LHS DAG 节点（如果已设置）派生，否则从 RHS DAG 节点派生。

---

**`!cond(cond1 : val1, cond2 : val2, ..., condn : valn)`**  
该运算符测试 cond1，如果结果为真则返回 val1。如果为假，则测试 cond2，如果结果为真则返回 val2。以此类推。如果没有条件为真，则报告错误。

此示例产生整数的符号词：

```tablegen
!cond(!lt(x, 0) : "negative", !eq(x, 0) : "zero", true : "positive")
```

---

**`!dag(op, arguments, names)`**  
该运算符创建具有给定运算符和参数的 DAG 节点。`arguments` 和 `names` 参数必须是等长的列表，或者是未初始化的（`?`）。`names` 参数必须是 `list<string>` 类型。

由于类型系统的限制，`arguments` 必须是具有公共类型的项目的列表。在实践中，这意味着它们应该具有相同的类型，或者是具有公共父类的记录。混合 `dag` 和非 `dag` 项目是不可能的。但是可以使用 `?`。

示例：`!dag(op, [a1, a2, ?], ["name1", "name2", "name3"])` 结果为 `(op a1-value:$name1, a2-value:$name2, ?:$name3)`。

---

**`!div(a, b)`**  
该运算符执行 a 除以 b 的有符号除法，并产生商。除以 0 产生错误。INT64_MIN 除以 -1 产生错误。

---

**`!empty(a)`**  
如果字符串、列表或 DAG a 为空，则产生 1；否则为 0。如果 DAG 没有参数，则该 DAG 为空；运算符不计入。

---

**`!eq(a, b)`**  
如果 a 等于 b，则产生 1；否则为 0。参数必须是 `bit`、`bits`、`int`、`string` 或记录值。使用 `!cast<string>` 比较其他类型的对象。

---

**`!exists<type>(name)`**  
如果存在名称为 name 的给定类型的记录，则产生 1；否则为 0。name 应为 `string` 类型。

---

**`!filter(var, list, predicate)`**  
该运算符通过过滤 list 中的元素来创建新列表。为了执行过滤，TableGen 将变量 var 绑定到每个元素，然后求值谓词表达式（谓词可能引用 var）。谓词必须产生布尔值（`bit`、`bits` 或 `int`）。该值的解释与 `!if` 相同：如果值为 0，则元素不包含在新列表中；如果值为其他任何值，则包含该元素。

---

**`!find(string1, string2[, start])`**  
该运算符在 string1 中搜索 string2 并产生其位置。搜索的起始位置可以由 start 指定，可以在 0 到 string1 的长度之间；默认为 0。如果未找到字符串，则结果为 -1。

---

**`!foldl(init, list, acc, var, expr)`**  
该运算符对 list 中的项目执行左折叠。变量 acc 充当累加器，初始化为 init。变量 var 绑定到列表中的每个元素。对每个元素求值表达式，表达式可能使用 acc 和 var 计算累积值，`!foldl` 将其存储回 acc 中。acc 的类型与 init 相同；var 的类型与 list 的元素相同；expr 必须与 init 具有相同的类型。

以下示例计算 RecList 中记录列表的 Number 字段的总和：

```tablegen
int x = !foldl(0, RecList, total, rec, !add(total, rec.Number));
```

如果你的目标是过滤列表并产生只包含部分元素的新列表，请参见 `!filter`。

---

**`!foreach(var, sequence, expr)`**  
该运算符创建新列表/DAG，其中每个元素是 sequence 列表/DAG 中对应元素的函数。为了执行该函数，TableGen 将变量 var 绑定到一个元素，然后求值表达式。表达式可能引用变量 var 并计算结果值。

如果你只是想创建一个包含多次重复相同值的特定长度的列表，请参见 `!listsplat`。

---

**`!ge(a, b)`**  
如果 a 大于或等于 b，则产生 1；否则为 0。参数必须是 `bit`、`bits`、`int` 或 `string` 值。

---

**`!getdagarg<type>(dag, key)`**  
该运算符通过指定的 key（整数索引或字符串名称）从给定 dag 节点检索参数。如果该参数无法转换为指定类型，则返回 `?`。

---

**`!getdagname(dag, index)`**  
该运算符通过指定的 index 从给定 dag 节点检索参数名称。如果该参数没有关联名称，则返回 `?`。

---

**`!getdagop(dag)` 或 `!getdagop<type>(dag)`**  
该运算符产生给定 dag 节点的运算符。示例：`!getdagop((foo 1, 2))` 结果为 `foo`。回想一下，DAG 运算符始终是记录。

`!getdagop` 的结果可以直接用于接受任何记录类的上下文（通常将其放入另一个 dag 值中）。但在其他上下文中，必须将其显式转换为特定类。提供 `<type>` 语法使这变得容易。

例如，要将结果赋给 `BaseClass` 类型的值，你可以写：

```tablegen
BaseClass b = !getdagop<BaseClass>(someDag);
BaseClass b = !cast<BaseClass>(!getdagop(someDag));
```

但要创建复用另一个节点运算符的新 DAG 节点，不需要强制转换：

```tablegen
dag d = !dag(!getdagop(someDag), args, names);
```

---

**`!getdagopname(dag)`**  
该运算符检索给定 dag 运算符的名称。如果运算符没有关联名称，则返回 `?`。

---

**`!gt(a, b)`**  
如果 a 大于 b，则产生 1；否则为 0。参数必须是 `bit`、`bits`、`int` 或 `string` 值。

---

**`!head(a)`**  
该运算符产生列表 a 的第零个元素。（另请参见 `!tail`。）

---

**`!if(test, then, else)`**  
该运算符求值 test，必须产生 `bit` 或 `int`。如果结果不为 0，则产生 `then` 表达式；否则产生 `else` 表达式。

---

**`!initialized(a)`**  
如果 a 不是未初始化值（`?`），则产生 1；否则为 0。

---

**`!instances<type>([regex])`**  
该运算符产生类型为 type 的记录列表。如果提供了 regex，则只包含名称匹配正则表达式 regex 的记录。regex 的格式为 ERE（扩展 POSIX 正则表达式）。

如果 `!instances` 在类/多类/foreach 中，则只考虑已实例化的该类型记录。

---

**`!interleave(list, delim)`**  
该运算符拼接列表中的项目，在每对之间插入 delim 字符串，并产生结果字符串。列表可以是 `string`、`int`、`bits` 或 `bit` 的列表。空列表产生空字符串。分隔符可以是空字符串。

---

**`!isa<type>(a)`**  
如果 a 的类型是给定类型的子类型，则产生 1；否则为 0。

---

**`!le(a, b)`**  
如果 a 小于或等于 b，则产生 1；否则为 0。参数必须是 `bit`、`bits`、`int` 或 `string` 值。

---

**`!listconcat(list1, list2, ...)`**  
该运算符拼接列表参数 list1、list2 等，产生结果列表。列表必须具有相同的元素类型。

---

**`!listflatten(list)`**  
该运算符展平列表的列表 list，产生包含所有构成列表元素的拼接列表。如果 list 的类型为 `list<list<X>>`，则结果列表类型为 `list<X>`。如果 list 的元素类型不是列表，则结果就是 list 本身。

---

**`!listremove(list1, list2)`**  
该运算符返回 list1 的副本，删除所有也出现在 list2 中的元素。列表必须具有相同的元素类型。

---

**`!listsplat(value, count)`**  
该运算符产生长度为 count 的列表，其所有元素都等于 value。例如，`!listsplat(42, 3)` 结果为 `[42, 42, 42]`。

---

**`!logtwo(a)`**  
该运算符产生 a 的以 2 为底的对数，并产生整数结果。0 或负数的对数产生错误。这是一个取整（向下）操作。

---

**`!lt(a, b)`**  
如果 a 小于 b，则产生 1；否则为 0。参数必须是 `bit`、`bits`、`int` 或 `string` 值。

---

**`!match(str, regex)`**  
如果 str 匹配正则表达式 regex，则产生 1。regex 的格式为 ERE（扩展 POSIX 正则表达式）。

---

**`!mul(a, b, ...)`**  
该运算符将 a、b 等相乘，产生乘积。

---

**`!ne(a, b)`**  
如果 a 不等于 b，则产生 1；否则为 0。参数必须是 `bit`、`bits`、`int`、`string` 或记录值。使用 `!cast<string>` 比较其他类型的对象。

---

**`!not(a)`**  
该运算符对 a 执行逻辑 NOT，a 必须是整数。参数 0 结果为 1（真）；任何其他参数结果为 0（假）。

---

**`!or(a, b, ...)`**  
该运算符对 a、b 等执行按位 OR，产生结果。如果所有参数都是 0 或 1，则可以执行逻辑 OR。当最左边的操作数为 -1 时，此运算符短路为 -1（全 1）。

---

**`!range([start,] end [,step])`**  
该运算符产生半开范围序列 `[start : end : step)` 作为 `list<int>`。start 默认为 0，step 默认为 1。step 可以为负数，但不能为 0。如果 start < end 且 step 为负数，或 start > end 且 step 为正数，则结果为空列表 `[]<int>`。

示例：
- `!range(4)` 等价于 `!range(0, 4, 1)`，结果为 `[0, 1, 2, 3]`。
- `!range(1, 4)` 等价于 `!range(1, 4, 1)`，结果为 `[1, 2, 3]`。
- `!range(0, 4, 2)` 的结果为 `[0, 2]`。
- `!range(0, 4, -1)` 和 `!range(4, 0, 1)` 的结果为空。

**`!range(list)`**  
等价于 `!range(0, !size(list))`。

---

**`!repr(value)`**  
将 value 表示为字符串。字符串格式不保证稳定。仅用于调试目的。

---

**`!setdagarg(dag, key, arg)`**  
该运算符产生与 dag 具有相同运算符和参数的 DAG 节点，但将 key 指定的参数值替换为 arg。key 可以是整数索引或字符串名称。

---

**`!setdagname(dag, key, name)`**  
该运算符产生与 dag 具有相同运算符和参数的 DAG 节点，但将 key 指定的参数名称替换为 name。key 可以是整数索引或字符串名称。

---

**`!setdagop(dag, op)`**  
该运算符产生与 dag 具有相同参数但运算符替换为 op 的 DAG 节点。示例：`!setdagop((foo 1, 2), bar)` 结果为 `(bar 1, 2)`。

---

**`!setdagopname(dag, name)`**  
该运算符产生与 dag 具有相同运算符和参数的 DAG 节点，但将运算符的名称替换为 name。

---

**`!shl(a, count)`**  
该运算符将 a 逻辑左移 count 位，并产生结果值。操作在 64 位整数上执行；对于 0…63 范围外的移位计数，结果未定义。

---

**`!size(a)`**  
该运算符产生字符串、列表或 DAG a 的大小。DAG 的大小是参数的数量；运算符不计入。

---

**`!sra(a, count)`**  
该运算符将 a 算术右移 count 位，并产生结果值。操作在 64 位整数上执行；对于 0…63 范围外的移位计数，结果未定义。

---

**`!srl(a, count)`**  
该运算符将 a 逻辑右移 count 位，并产生结果值。操作在 64 位整数上执行；对于 0…63 范围外的移位计数，结果未定义。

---

**`!strconcat(str1, str2, ...)`**  
该运算符拼接字符串参数 str1、str2 等，产生结果字符串。

---

**`!sub(a, b)`**  
该运算符从 a 中减去 b，产生算术差。

---

**`!subst(target, repl, value)`**  
该运算符将 value 中所有出现的 target 替换为 repl，并产生结果值。value 可以是字符串，在这种情况下执行子字符串替换。value 可以是记录名称，在这种情况下如果 target 记录名称等于 value 记录名称，则运算符产生 repl 记录；否则产生 value。

---

**`!substr(string, start[, length])`**  
该运算符提取给定字符串的子字符串。子字符串的起始位置由 start 指定，可以在 0 到字符串长度之间。子字符串的长度由 length 指定；如果未指定，则提取字符串的其余部分。start 和 length 参数必须是整数。

---

**`!tail(a)`**  
该运算符产生包含列表 a 除第零个元素以外所有元素的新列表。（另请参见 `!head`。）

---

**`!tolower(a)`**  
该运算符将字符串输入 a 转换为小写。

---

**`!toupper(a)`**  
该运算符将字符串输入 a 转换为大写。

---

**`!xor(a, b, ...)`**  
该运算符对 a、b 等执行按位异或（EXCLUSIVE OR），产生结果。如果所有参数都是 0 或 1，则可以执行逻辑 XOR。

---

## 11 附录 B：粘贴运算符示例

以下示例说明了粘贴运算符在记录名称中的使用：

```tablegen
defvar suffix = "_suffstring";
defvar some_ints = [0, 1, 2, 3];

def name # suffix {
}

foreach i = [1, 2] in {
def rec # i {
}
}
```

第一个 `def` 不使用 `suffix` 变量的值。第二个 `def` 使用迭代变量 `i` 的值，因为它不是全局名称。产生以下记录：

```
def namesuffix {
}
def rec1 {
}
def rec2 {
}
```

以下是粘贴运算符在字段值表达式中的第二个示例：

```tablegen
def test {
  string strings = suffix # suffix;
  list<int> integers = some_ints # [4, 5, 6];
}
```

`strings` 字段表达式在粘贴运算符两侧都使用 `suffix`。左侧正常求值，右侧逐字处理。`integers` 字段表达式使用 `some_ints` 变量的值和字面列表。产生以下记录：

```
def test {
  string strings = "_suffstringsuffix";
  list<int> ints = [0, 1, 2, 3, 4, 5, 6];
}
```

---

## 12 附录 C：示例记录

LLVM 支持的一个目标机器是 Intel x86。以下来自 TableGen 的输出显示了为表示 32 位寄存器到寄存器 ADD 指令而创建的记录：

```
def ADD32rr { // InstructionEncoding Instruction X86Inst I ITy Sched BinOpRR BinOpRR_RF
  int Size = 0;
  string DecoderNamespace = "";
  list<Predicate> Predicates = [];
  string DecoderMethod = "";
  bit hasCompleteDecoder = 1;
  string Namespace = "X86";
  dag OutOperandList = (outs GR32:$dst);
  dag InOperandList = (ins GR32:$src1, GR32:$src2);
  string AsmString = "add{l}  {$src2, $src1|$src1, $src2}";
  EncodingByHwMode EncodingInfos = ?;
  list<dag> Pattern = [(set GR32:$dst, EFLAGS, (X86add_flag GR32:$src1, GR32:$src2))];
  list<Register> Uses = [];
  list<Register> Defs = [EFLAGS];
  int CodeSize = 3;
  int AddedComplexity = 0;
  bit isPreISelOpcode = 0;
  bit isReturn = 0;
  bit isBranch = 0;
  ... （共 109 个字段）
}
```

在记录的第一行，你可以看到 `ADD32rr` 记录继承自八个类。尽管继承层次结构很复杂，使用父类比为每条指令单独指定 109 个字段要简单得多。

以下是用于定义 `ADD32rr` 和多条其他 ADD 指令的代码片段：

```tablegen
defm ADD : ArithBinOp_RF<0x00, 0x02, 0x04, "add", MRM0r, MRM0m,
                         X86add_flag, add, 1, 1, 1>;
```

`defm` 语句告诉 TableGen `ArithBinOp_RF` 是一个多类，其中包含多个继承自 `BinOpRR_RF` 的具体记录定义。该类又继承自 `BinOpRR`，后者继承自 `ITy` 和 `Sched`，依此类推。字段从所有父类继承；例如，`IsIndirectBranch` 从 `Instruction` 类继承。

---

*© Copyright 2003-2026, LLVM Project. 最后更新于 2026-04-22。*
