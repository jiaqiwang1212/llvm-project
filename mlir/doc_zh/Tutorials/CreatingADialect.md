# 创建方言

[TOC]

公共方言通常被分散在至少 3 个目录中：
* mlir/include/mlir/Dialect/Foo   （用于公共头文件）
* mlir/lib/Dialect/Foo            （用于源文件）
* mlir/lib/Dialect/Foo/IR         （用于操作）
* mlir/lib/Dialect/Foo/Transforms （用于变换）
* mlir/test/Dialect/Foo           （用于测试）

除其他公共头文件外，`include` 目录还包含一个 [ODS 格式](../DefiningDialects/Operations.md) 的 TableGen 文件，用于描述方言中的操作。该文件用于生成操作声明（FooOps.h.inc）和定义（FooOps.cpp.inc），以及操作接口声明（FooOpsInterfaces.h.inc）和定义（FooOpsInterfaces.cpp.inc）。

`IR` 目录通常包含方言中未由 ODS 自动生成的函数实现。这些函数通常定义在 FooDialect.cpp 中，该文件包含 FooOps.cpp.inc 和 FooOpsInterfaces.h.inc。

`Transforms` 目录包含该方言的重写规则，通常使用 [DRR 格式](../DeclarativeRewrites.md) 在 TableGen 文件中描述。

请注意，方言名称通常不应以 "Ops" 作为后缀，但与方言操作相关的某些文件（例如 FooOps.cpp）可能会使用该后缀。

## CMake 最佳实践

### TableGen 目标

方言中的操作通常使用 ODS 格式在 FooOps.td 文件中通过 tablegen 声明。该文件构成方言的核心，并使用 add_mlir_dialect() 进行声明。

```cmake
add_mlir_dialect(FooOps foo)
add_mlir_doc(FooOps FooDialect Dialects/ -gen-dialect-doc)
```

这会生成运行 mlir-tblgen 所需的正确规则，以及可用于声明依赖项的 `MLIRFooOpsIncGen` 目标。

方言变换通常在 FooTransforms.td 文件中声明。TableGen 的目标按照典型的 LLVM 方式描述。

```cmake
set(LLVM_TARGET_DEFINITIONS FooTransforms.td)
mlir_tablegen(FooTransforms.h.inc -gen-rewriters)
add_public_tablegen_target(MLIRFooTransformIncGen)
```

结果是另一个 `IncGen` 目标，它运行 mlir-tblgen。

### 库目标

方言可以有多个库。每个库通常使用 add_mlir_dialect_library() 进行声明。方言库通常依赖于从 TableGen 生成的头文件（使用 DEPENDS 关键字指定）。方言库也可能依赖于其他方言库。这种依赖关系通常使用 target_link_libraries() 和 PUBLIC 关键字来声明。例如：

```cmake
add_mlir_dialect_library(MLIRFoo
  DEPENDS
  MLIRFooOpsIncGen
  MLIRFooTransformsIncGen

  LINK_COMPONENTS
  Core

  LINK_LIBS PUBLIC
  MLIRBar
  <some-other-library>
  )
```

add_mlir_dialect_library() 是对 add_llvm_library() 的轻量封装，它收集所有方言库的列表。该列表通常对链接工具（例如 mlir-opt）很有用，这些工具应该能够访问所有方言。该列表也被链接到 libMLIR.so 中。可以通过 MLIR_DIALECT_LIBS 全局属性来获取该列表：

```cmake
get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
```

请注意，虽然 Bar 方言也使用 TableGen 来声明其操作，但不必显式依赖对应的 IncGen 目标。PUBLIC 链接依赖已经足够。还请注意，我们避免显式使用 add_dependencies，因为依赖项需要对底层的 add_llvm_library() 调用可用，以便它能够正确地使用相同的源文件创建新目标。但是，依赖 LLVM IR 的方言可能需要依赖 LLVM 的 `intrinsics_gen` 目标，以确保 tablegen 生成的 LLVM 头文件已经生成。

此外，与 MLIR 库的链接使用 LINK_LIBS 描述符指定，与 LLVM 库的链接使用 LINK_COMPONENTS 描述符指定。这允许 cmake 基础设施生成具有正确链接的新库目标，特别是在指定 BUILD_SHARED_LIBS=on 或 LLVM_LINK_LLVM_DYLIB=on 时。


# 方言转换

从 "X" 到 "Y" 的转换分别位于 mlir/include/mlir/Conversion/XToY、mlir/lib/Conversion/XToY 和 mlir/test/Conversion/XToY 中。

转换的默认文件名应从名称中省略 "Convert"，例如 lib/VectorToLLVM/VectorToLLVM.cpp。

转换通道（pass）应与转换本身分开存放，以方便只关心通道而不关心其模式或其他基础设施实现的用户。例如 include/mlir/VectorToLLVM/VectorToLLVMPass.h。

不属于方言定义的通用转换功能（来自或到方言 "X"）可以放在 mlir/lib/Conversion/XCommon 中，例如 mlir/lib/Conversion/GPUCommon。

## CMake 最佳实践

每个转换通常存在于一个单独的库中，使用 add_mlir_conversion_library() 声明。转换库通常依赖于其源方言和目标方言，但也可能依赖于其他方言（例如 MLIRFunc）。这种依赖关系通常使用 target_link_libraries() 和 PUBLIC 关键字来指定。例如：

```cmake
add_mlir_conversion_library(MLIRBarToFoo
  BarToFoo.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/BarToFoo

  LINK_LIBS PUBLIC
  MLIRBar
  MLIRFoo
  )
```

add_mlir_conversion_library() 是对 add_llvm_library() 的轻量封装，它收集所有转换库的列表。该列表通常对链接工具（例如 mlir-opt）很有用，这些工具应该能够访问所有方言。该列表也被链接到 libMLIR.so 中。可以通过 MLIR_CONVERSION_LIBS 全局属性来获取该列表：

```cmake
get_property(dialect_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
```

请注意，只需要针对方言指定 PUBLIC 依赖项来生成编译时和链接时依赖关系，不必显式依赖方言的 IncGen 目标。但是，直接包含 LLVM IR 头文件的转换可能需要依赖 LLVM 的 `intrinsics_gen` 目标，以确保 tablegen 生成的 LLVM 头文件已经生成。
