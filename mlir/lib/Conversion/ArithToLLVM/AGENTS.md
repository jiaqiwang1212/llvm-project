<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToLLVM Conversion

## Purpose
Lowers the Arith dialect to LLVM IR dialect ops. The primary lowering path for all integer and floating-point arithmetic, comparisons, bitwise ops, and casts to their LLVM IR equivalents.

## Key Files
| File | Description |
|------|-------------|
| `ArithToLLVM.cpp` | All conversion patterns: arith ops → llvm ops |

## For AI Agents

### Working In This Directory
- Direct 1:1 mappings: `arith.addi` → `llvm.add`, `arith.mulf` → `llvm.fmul`, `arith.cmpi` → `llvm.icmp`, etc.
- `arith.constant` → `llvm.mlir.constant` with type conversion via `LLVMTypeConverter`.
- Vector arith ops are handled naturally since LLVM dialect supports the same vector types.
- Uses `ArithCommon/AttrToLLVMConverter` for attribute value conversion.
- Integer overflow flags (`nsw`, `nuw`) are carried from arith op attributes.

## Dependencies
- Source: `mlir/Dialect/Arith`
- Target: `mlir/Dialect/LLVMIR`
- Utility: `mlir/Conversion/LLVMCommon`

<!-- MANUAL: -->
