<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithCommon Conversion

## Purpose
Provides shared utilities for converting Arith dialect attribute values (integer and float attributes) to LLVM IR attributes. Used by multiple Arith conversion passes as common infrastructure.

## Key Files
| File | Description |
|------|-------------|
| `AttrToLLVMConverter.cpp` | Converts `IntegerAttr` and `FloatAttr` to LLVM dialect attributes |

## For AI Agents

### Working In This Directory
- `AttrToLLVMConverter.cpp` handles the mapping of MLIR `IntegerAttr`/`FloatAttr` to their LLVM-dialect equivalents including special float values (inf, nan).
- This is a utility library — not a standalone pass. It is linked into `ArithToLLVM` and other Arith conversion passes.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/Dialect/LLVMIR`

<!-- MANUAL: -->
