# MLIR 字节码格式

本文档描述了 MLIR 字节码(bytecode)格式及其编码方式。该格式具有版本控制且保持稳定：我们不打算破坏兼容性，即方言(dialect)应能反序列化任何旧版字节码。同样，我们支持向后部署，以便可以针对旧版格式。

需要注意的是，字节码格式的承诺是在假定方言不可变的前提下作出的：该格式允许向前和向后兼容，但仅限于方言中没有任何变化（操作(op)、类型、属性(attribute)定义）的情况。

方言可以通过 `BytecodeDialectInterface` 来选择自行处理版本控制。该接口向方言暴露了一些钩子，用于管理编码到字节码文件中的版本。版本会延迟加载，允许在解码输入 IR(中间表示) 时检索版本信息，并通过 `upgradeFromVersion` 方法为每个存在版本的方言提供在解析后进行 IR 升级的机会。对于方言可以编码哪些信息来对其版本进行建模，没有任何限制。

[TOC]

## 魔数

MLIR 使用以下四字节魔数来标识字节码文件：

'\['M'<sub>8</sub>, 'L'<sub>8</sub>, 'ï'<sub>8</sub>, 'R'<sub>8</sub>\]'

十六进制表示：

'\['4D'<sub>8</sub>, '4C'<sub>8</sub>, 'EF'<sub>8</sub>, '52'<sub>8</sub>\]'

## 格式概览

MLIR 字节码文件由一个字节流组成，在其上叠加了一些简单的结构概念。

### 原语

#### 固定宽度整数

```
  byte ::= `0x00`...`0xFF`
```

固定宽度整数是已知字节大小的无符号整数。值以小端字节序存储。

TODO: 根据需要添加更大的固定宽度整数。

#### 变长整数

变长整数（`VarInt`）为整数提供了一种紧凑的表示方式。每个编码的 VarInt 由一到九个字节组成，这些字节共同表示一个 64 位的值。MLIR 字节码使用 "PrefixVarInt" 编码方式对 VarInt 进行编码。这种编码是
[LEB128（"小端基 128"）](https://en.wikipedia.org/wiki/LEB128)
编码的变体，其中编码的每个字节最多提供 7 位用于存值，剩余 1 位用于存储一个标记，指示编码所用的字节数。这意味着小的无符号整数（小于 2^7）可以用一个字节存储，最大 2^14 的无符号整数可以用两个字节存储，以此类推。

编码的第一个字节在低位包含一个长度前缀。这个前缀是由一串 '0' 位后跟一个终止 '1' 位组成的比特序列，或者直到字节末尾。'0' 位的数量表示编码所用的额外字节数（不包括前缀字节）。第一个字节中的所有其余位，以及额外字节中的所有位，共同提供整数的值。以下是前缀字节的各种可能编码：

```
xxxxxxx1:  7 value bits, the encoding uses 1 byte
xxxxxx10: 14 value bits, the encoding uses 2 bytes
xxxxx100: 21 value bits, the encoding uses 3 bytes
xxxx1000: 28 value bits, the encoding uses 4 bytes
xxx10000: 35 value bits, the encoding uses 5 bytes
xx100000: 42 value bits, the encoding uses 6 bytes
x1000000: 49 value bits, the encoding uses 7 bytes
10000000: 56 value bits, the encoding uses 8 bytes
00000000: 64 value bits, the encoding uses 9 bytes
```

##### 有符号变长整数

有符号变长整数值的编码方式与[变长整数](#变长整数)类似，但采用了
[之字形编码（zigzag encoding）](https://en.wikipedia.org/wiki/Variable-length_quantity#Zigzag_encoding)。
该编码使用值的低位来表示符号，从而能更高效地编码负数。如果负数使用普通[变长整数](#变长整数)编码，它将被视为一个极大的无符号值。使用之字形编码可以减少值中的有效位数，从而得到更小的编码。以下是生成之字形编码的基本计算方式：

```
(value << 1) ^ (value >> 63)
```

#### 字符串

字符串是带有关联长度的字符数据块。

### 节

```
section {
  idAndIsAligned: byte // id | (hasAlign << 7)
  length: varint,

  alignment: varint?,
  padding: byte[], // Padding bytes are always `0xCB`.

  data: byte[]
}
```

节(section)是字节码中用于分组数据的机制。它们支持延迟处理，这对于乱序处理数据、延迟加载等场景非常有用。每个节包含一个节 ID（其最高位指示该节是否有对齐要求）、一个长度（允许跳过该节）以及一个可选的对齐值。当存在对齐时，节数据之前可能会出现可变数量的填充字节（0xCB）。节的对齐必须是 2 的幂次方。输入字节码缓冲区必须满足与每个节相同的对齐要求。

## MLIR 编码

鉴于 MLIR 的通用结构，字节码编码实际上相当简单。它有效地映射到 MLIR 的核心组件。

### 顶层结构

字节码的顶层结构包含 4 字节的"魔数"、一个版本号、一个以空字符结尾的生产者字符串以及一个节列表。目前每个节在字节码文件中只应出现一次。

```
bytecode {
  magic: "MLïR",
  version: varint,
  producer: string,
  sections: section[]
}
```

### 字符串节

```
strings {
  numStrings: varint,
  reverseStringLengths: varint[],
  stringData: byte[]
}
```

字符串节包含字节码中引用的字符串表，从而更容易地实现字符串共享。该节首先编码字符串总数，然后以逆序编码每个字符串的大小。其余编码包含将所有字符串连接在一起的单个数据块。

### 方言节

字节码的方言节包含编码后的 IR 中引用的所有方言，以及这些方言中同样被引用的组件的一些信息。

```
dialect_section {
  numDialects: varint,
  dialectNames: dialect_name_group[],
  opNames: dialect_ops_group[]  // ops grouped by dialect
}

dialect_name_group {
  nameAndIsVersioned: varint  // (dialectID << 1) | (hasVersion),
  version: dialect_version_section  // only if versioned
}

dialect_version_section {
  size: varint,
  version: byte[]
}

dialect_ops_group {
  dialect: varint,
  numOpNames: varint,
  opNames: op_name_group[]
}

op_name_group {
  nameAndIsRegistered: varint  // (nameID << 1) | (isRegisteredOp)
}
```

方言被编码为一个 `varint`，包含字符串节中名称字符串的索引，以及一个指示该方言是否有版本控制的标志。操作名称按方言分组编码，每组包含方言、操作名称数量以及字符串节中每个名称的索引数组。版本作为每个方言的嵌套节进行编码。

### 属性/类型节

属性和类型使用两个[节](#节)进行编码，一个节（`attr_type_section`）包含实际的编码表示，另一个节（`attr_type_offset_section`）包含每个编码属性/类型在前一个节中的偏移量。这种结构允许属性和类型始终按需延迟加载。

```
attr_type_section {
  attrs: attribute[],
  types: type[]
}
attr_type_offset_section {
  numAttrs: varint,
  numTypes: varint,
  offsets: attr_type_offset_group[]
}

attr_type_offset_group {
  dialect: varint,
  numElements: varint,
  offsets: varint[] // (offset << 1) | (hasCustomEncoding)
}

attribute {
  encoding: ...
}
type {
  encoding: ...
}
```

`attr_type_offset_section` 中的每个 `offset` 是属性或类型编码的大小，以及一个指示编码是否使用文本汇编格式或自定义字节码编码的标志。我们避免使用 `attr_type_section` 中的直接偏移，因为较小的相对偏移量能提供更有效的压缩。属性和类型按方言分组，偏移节中的每个 `attr_type_offset_group` 包含对应的父方言、元素数量以及组内每个元素的偏移量。

#### 属性/类型编码

从抽象层面来看，属性/类型以两种可能的方式之一进行编码：通过其汇编格式，或通过自定义的方言定义编码。

##### 汇编格式回退

当方言没有为属性或类型定义编码方法时，该属性或类型的文本汇编格式将被用作回退。例如，类型 `!bytecode.type<42>` 将被编码为以空字符结尾的字符串 "!bytecode.type<42>"。这确保了每个属性和类型都可以被编码，即使所属方言尚未选择更高效的序列化方式。

TODO: 这里不应该冗余地编码方言名称，应该使用对父方言的引用代替。

##### 方言自定义编码

作为汇编格式回退的替代方案，方言也可以为其属性和类型提供自定义编码。自定义编码非常有益，因为它们读写速度明显更快且体积更小。

方言可以通过实现 `BytecodeDialectInterface` 来选择提供自定义编码。该接口提供了钩子，即 `readAttribute`/`readType` 和 `writeAttribute`/`writeType`，这些钩子将被字节码读写器使用。这些钩子提供了读写器实现，可用于在底层字节码格式中编码各种结构。该接口的一个独特功能是，方言可以选择只以自定义字节码格式编码其属性和类型的子集，这简化了尚未完全成熟的新实验性组件的添加。

实现字节码接口时，方言负责编码的所有方面。这包括指示正在编码的是哪种属性或类型的指示符；字节码读取器只知道它遇到了某个方言的属性或类型，不会编码任何进一步的信息。因此，一种常见的编码习惯是使用一个前导 `varint` 代码来指示属性或类型的编码方式。

### 资源节

资源使用两个[节](#节)进行编码，一个节（`resource_section`）包含实际的编码表示，另一个节（`resource_offset_section`）包含每个编码资源在前一个节中的偏移量。

```
resource_section {
  resources: resource[]
}
resource {
  value: resource_bool | resource_string | resource_blob
}
resource_bool {
  value: byte
}
resource_string {
  value: varint
}
resource_blob {
  alignment: varint,
  size: varint,
  padding: byte[],
  blob: byte[]
}

resource_offset_section {
  numExternalResourceGroups: varint,
  resourceGroups: resource_group[]
}
resource_group {
  key: varint,
  numResources: varint,
  resources: resource_info[]
}
resource_info {
  key: varint,
  size: varint
  kind: byte,
}
```

资源按提供者（外部实体或方言）分组，偏移节中的每个 `resource_group` 包含对应的提供者、元素数量以及组内每个元素的信息。对于每个元素，我们记录键、值类型和编码大小。我们避免使用 `resource_section` 中的直接偏移，因为较小的相对偏移量能提供更有效的压缩。

### IR 节

IR 节包含字节码中操作的编码形式。

```
ir_section {
  block: block; // Single block without arguments.
}
```

#### 操作编码

```
op {
  name: varint,
  encodingMask: byte,
  location: varint,

  attrDict: varint?,

  numResults: varint?,
  resultTypes: varint[],

  numOperands: varint?,
  operands: varint[],

  numSuccessors: varint?,
  successors: varint[],

  numUseListOrders: varint?,
  useListOrders: uselist[],

  regionEncoding: varint?, // (numRegions << 1) | (isIsolatedFromAbove)

  // regions are stored in a section if isIsolatedFromAbove
  regions: (region | region_section)[]
}

uselist {
  indexInRange: varint?,
  useListEncoding: varint, // (numIndices << 1) | (isIndexPairEncoding)
  indices: varint[]
}
```

操作编码非常重要，因为这通常是字节码中出现最频繁的结构。对于每种类型的操作都使用单一的编码。鉴于其普遍性，操作的许多字段都是可选的。`encodingMask` 字段是一个位掩码，指示操作中哪些组件存在。

##### 位置

位置被编码为属性表中该位置的索引。

##### 属性

如果操作有属性，则编码操作属性字典在属性表中的索引。

##### 结果

如果操作有结果，则编码结果数量以及类型表中每个结果类型的索引。

##### 操作数

如果操作有操作数，则编码操作数数量以及每个操作数的值索引。该值索引是该值的定义相对于第一个祖先隔离区域起始处的相对顺序。

##### 后继

如果操作有后继，则编码后继数量以及父区域中每个后继基本块的索引。

##### 使用列表顺序

参考使用列表顺序被假定为对 IR 进行前序遍历时所获得的所有操作数的全局枚举的逆序。这种顺序在逐操作构建基本块时自然产生。然而，某些变换可能会相对于此参考顺序打乱使用列表。如果操作的任何结果的使用列表顺序相对于参考使用列表顺序不是有序的，则会发出一种编码，使得在解析字节码后可以重建这种顺序。该编码表示从参考操作数顺序到当前使用列表顺序的索引映射。使用一个位标志来检测该编码是否为索引对类型。当位标志设置为零时，位置 `i` 的元素表示参考列表中使用 `i` 在当前使用列表中的位置。当位标志设置为 `1` 时，编码表示索引对 `(i, j)`，指示参考列表中位置 `i` 的使用映射到当前使用列表中的位置 `j`。当当前使用列表中只有不到一半的元素相对于参考使用列表被打乱时，使用索引对编码来减少字节码的内存需求。

##### 区域

如果操作有区域，则将区域数量和区域是否与上方隔离这两个信息一起编码到单个 varint 中。之后，每个区域以内联方式编码。

#### 区域编码

```
region {
  numBlocks: varint,

  numValues: varint?,
  blocks: block[]
}
```

区域首先编码其中的基本块数量。如果区域非空，则编码直接在区域内定义的值的数量，然后是区域中的各个基本块。

#### 基本块编码

```
block {
  encoding: varint, // (numOps << 1) | (hasBlockArgs)
  arguments: block_arguments?, // Optional based on encoding
  ops : op[]
}

block_arguments {
  numArgs: varint?,
  args: block_argument[]
  numUseListOrders: varint?,
  useListOrders: uselist[],
}

block_argument {
  typeAndLocation: varint, // (type << 1) | (hasLocation)
  location: varint? // Optional, else unknown location
}
```

基本块由一个操作数组和基本块参数编码而成。第一个字段是一个编码，将基本块中的操作数量与一个指示基本块是否有参数的标志组合在一起。

使用列表顺序附加到基本块参数的方式与附加到操作结果的方式类似。
