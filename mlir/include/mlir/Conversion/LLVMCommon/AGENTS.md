<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LLVMCommon

## Purpose
Provides shared infrastructure for all LLVM IR conversion passes. Contains the common type converter, conversion patterns, memref/struct builders, and lowering options used across every `*ToLLVM` pass.

## Key Files
| File | Description |
|------|-------------|
| `ConversionTarget.h` | LLVM conversion target configuration helpers |
| `LoweringOptions.h` | Shared lowering option structs (data layout, index width, etc.) |
| `MemRefBuilder.h` | MemRef descriptor builder utilities for LLVM struct representation |
| `Pattern.h` | Base rewrite pattern classes for LLVM dialect lowering |
| `PrintCallHelper.h` | Helpers for emitting printf-like debug calls via LLVM dialect |
| `StructBuilder.h` | Generic LLVM struct value builder utilities |
| `TypeConverter.h` | The core `LLVMTypeConverter` shared by all LLVM lowering passes |
| `VectorPattern.h` | Vector type lowering pattern utilities for LLVM conversion |

## For AI Agents

### Working In This Directory
- `TypeConverter.h` is the most critical file — it converts MLIR types to LLVM types
- Edit pattern base classes in `Pattern.h` for changes affecting all LLVM lowering patterns
- Do not register standalone passes here; this is a shared utility library

## Dependencies
- Target dialect: `include/mlir/Dialect/LLVMIR/`
- Consumed by: all `*ToLLVM` conversion passes

<!-- MANUAL: -->
