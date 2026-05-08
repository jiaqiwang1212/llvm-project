<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlowToLLVM Conversion

## Purpose
Lowers the ControlFlow dialect to LLVM IR dialect. Converts `cf.br` → `llvm.br`, `cf.cond_br` → `llvm.cond_br`, `cf.switch` → `llvm.switch`, and `cf.assert` → conditional `llvm.call @abort`.

## Key Files
| File | Description |
|------|-------------|
| `ControlFlowToLLVM.cpp` | All conversion patterns from cf ops to llvm branch ops |

## For AI Agents

### Working In This Directory
- Direct 1:1 mappings for branch ops — block argument types must already be LLVM-compatible before this runs.
- `cf.assert` lowers to: compute condition, `llvm.cond_br` to abort block, abort block calls `llvm.call @abort()`.
- Must run after `ArithToLLVM` and other type-converting passes so branch block argument types are LLVM types.
- Uses `LLVMTypeConverter` from `LLVMCommon` for any type mappings.

## Dependencies
- Source: `mlir/Dialect/ControlFlow`
- Target: `mlir/Dialect/LLVMIR`
- Utility: `mlir/Conversion/LLVMCommon`

<!-- MANUAL: -->
