<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/LLVMIR/Transforms/

## Purpose
Pre-translation transformation pass headers for LLVM IR export. Contains passes that prepare MLIR IR for translation to LLVM IR (e.g., adding debug info, normalizing calling conventions, inserting target-specific attributes).

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Registration and factory functions for LLVMIR translation preparation passes |
| `Passes.td` | TableGen definitions for LLVMIR translation passes |
| `TargetUtils.h` | Target-specific utilities used during LLVM IR translation passes |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- These passes run immediately before `translateModuleToLLVMIR()` in a standard lowering pipeline.
- Example passes: `createDIExpressionLegalizationPass()`, `createSetLLVMModuleDataLayoutPass()`.
- Register via `registerTransformsPasses()` from `Passes.h`.

### Common Patterns
- Include in lowering pipeline: `pm.addPass(mlir::LLVM::createDIExpressionLegalizationPass());`

## Dependencies

### Internal
- `mlir/Dialect/LLVMIR/` (LLVM dialect ops)
- `mlir/Pass/` (Pass base class)

### External
- `llvm/IR/` (DataLayout, TargetMachine)

<!-- MANUAL: -->
