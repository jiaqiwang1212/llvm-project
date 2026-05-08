<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRefToEmitC

## Purpose
Implements the MemRef to EmitC lowering pass. Converts `memref` allocation, deallocation, load, and store operations into EmitC ops suitable for C code emission.

## Key Files
| File | Description |
|------|-------------|
| `MemRefToEmitC.cpp` | Core conversion patterns for memref ops to EmitC |
| `MemRefToEmitCPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateMemRefToEmitCConversionPatterns()`
- TypeConverter maps `memref` types to EmitC array/pointer types

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- EmitC targets C-level constructs; avoid LLVM dialect dependencies

## Dependencies
- Headers: `include/mlir/Conversion/MemRefToEmitC/`
- Source dialect: `lib/Dialect/MemRef/`
- Target dialect: `lib/Dialect/EmitC/`

<!-- MANUAL: -->
