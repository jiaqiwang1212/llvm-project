EmitC 方言（dialect）允许将其他 MLIR 方言（dialect）中的操作（op）转换为 EmitC 操作。这些操作可通过 Cpp 发射器（emitter）翻译为 C/C++ 代码。

遵循以下约定：

*   若向 `emitc.call_opaque` 操作传递模板参数，则生成 C++ 代码。
*   若使用张量（tensor），则生成 C++ 代码。
*   若在函数或 `emitc.call_opaque` 操作中使用多个返回值，则需要 C++11。
*   若向 `emitc.call_opaque` 操作传递浮点类型模板参数，则需要 C++20。
*   若使用 `ssize_t`，则代码需要 POSIX 头文件 `sys/types.h` 或任何定义该类型的 C++ 头文件。
*   若使用 `_Float16`，则代码需要支持 C 附加浮点类型。
*   若使用 `__bf16`，则代码需要支持该类型的编译器，如 GCC 或 Clang。
*   若使用零维度大小的 `emitc.array`，则代码需要 [GCC 扩展](https://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html)。
*   若向 `emitc.call_opaque` 操作传递 `aligned_alloc`，则需要 C++17 或 C11。
*   否则，生成的代码兼容 C99。

这些限制既不是 EmitC 方言本身固有的，也不是 Cpp 发射器固有的，因此在实现转换时需要加以考虑。

MLIR 类型 `index` 到无符号类型 `size_t` 及其有符号对应类型 `ptrdiff_t` 的类型转换已提供。这两种类型之间的转换仅在 `index` 类型的值位于 `[PTRDIFF_MIN, PTRDIFF_MAX]` 范围内时才有效。

转换完成后，可使用 `mlir-translate` 工具发射 C/C++ 代码。该工具支持通过传递 `-mlir-to-cpp` 将 MLIR 翻译为 C/C++。此外，通过传递附加参数 `-declare-variables-at-top`，可以生成在顶部声明变量的代码。

除 EmitC 方言中的操作外，Cpp 目标还支持翻译以下操作：

*   'cf' 方言
    *   `cf.br`
    *   `cf.cond_br`
*   'func' 方言
    *   `func.call`
    *   `func.func`
    *   `func.return`
