<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FuncToLLVM Conversion

## Purpose
Lowers the Func dialect to LLVM IR dialect. Converts `func.func` → `llvm.func`, `func.call` → `llvm.call`, `func.return` → `llvm.return`, and handles function signature conversion including C wrapper generation.

## Key Files
| File | Description |
|------|-------------|
| `FuncToLLVM.cpp` | All conversion patterns including signature rewriting and C wrapper generation |

## For AI Agents

### Working In This Directory
- `FuncOpConversion` uses `LLVMTypeConverter` to rewrite function type — multi-result functions become single-result struct-returning functions.
- C wrappers (`--emit-c-wrappers` option): generates `_mlir_ciface_*` wrapper functions with C-ABI-compatible signatures for calling from C.
- `func.func` with `llvm.emit_c_interface` attribute gets a C wrapper that takes `StridedMemRefType` structs for memref args.
- Must run after all type-converting passes since it consumes LLVM-compatible types.

## Dependencies
- Source: `mlir/Dialect/Func`
- Target: `mlir/Dialect/LLVMIR`
- Utility: `mlir/Conversion/LLVMCommon`

<!-- MANUAL: -->
