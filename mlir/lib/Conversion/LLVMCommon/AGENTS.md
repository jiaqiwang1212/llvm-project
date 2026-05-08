<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LLVMCommon Conversion

## Purpose
Provides shared infrastructure for all LLVM IR dialect conversions. Contains `LLVMTypeConverter` (the central MLIR→LLVM type mapping engine), `MemRefDescriptor` (the runtime struct for memref lowering), pattern base classes, and utility functions used by all `*ToLLVM` conversions.

## Key Files
| File | Description |
|------|-------------|
| `TypeConverter.cpp` | `LLVMTypeConverter`: converts MLIR types to LLVM types (memref→struct, index→iN, etc.) |
| `Pattern.cpp` | `ConvertOpToLLVMPattern` base class for all LLVM conversion patterns |
| `MemRefBuilder.cpp` | `MemRefDescriptor`: builds/decomposes the LLVM struct representing a memref at runtime |
| `StructBuilder.cpp` | Generic struct value builder/extractor for LLVM struct types |
| `ConversionTarget.cpp` | `LLVMConversionTarget`: configures which dialects/ops are legal after LLVM lowering |
| `LoweringOptions.cpp` | `LowerToLLVMOptions`: controls data layout, index width, use-opaque-pointers, etc. |
| `VectorPattern.cpp` | Shared patterns for lowering vector ops to LLVM |
| `PrintCallHelper.cpp` | Helper for emitting `printf`-like calls from LLVM dialect |

## For AI Agents

### Working In This Directory
- `LLVMTypeConverter` is the entry point for all MLIR→LLVM type mapping: `convertType(mlirType)` → LLVM type.
- `MemRefDescriptor` encodes a `memref<NxMxf32>` as an LLVM struct: `{ptr, ptr, i64, [N x i64], [N x i64]}` (allocated_ptr, aligned_ptr, offset, sizes, strides).
- `LowerToLLVMOptions` must be configured before creating `LLVMTypeConverter` — it controls whether index is i32/i64 and pointer representation.
- `ConvertOpToLLVMPattern<SourceOp>`: all concrete LLVM conversion patterns inherit from this; it provides `typeConverter`, `rewriter`, `matchAndRewrite`.
- `LLVMConversionTarget` marks all LLVM dialect ops as legal and all ops with LLVM-convertible types as illegal.

### Common Patterns
- Implementing a new `*ToLLVM` pattern: inherit `ConvertOpToLLVMPattern<MyOp>`, implement `matchAndRewrite`, use `typeConverter->convertType()` for types and `MemRefDescriptor` for memref operands.

## Dependencies
- `mlir/Dialect/LLVMIR`, `mlir/Transforms/DialectConversion`

<!-- MANUAL: -->
