<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSVE Dialect

## Purpose
Implements the Arm Scalable Vector Extension (SVE) dialect. SVE provides length-agnostic SIMD with `vscale`-based vector sizes. This dialect exposes SVE-specific ops (predicated operations, scalable reductions) not covered by the generic Vector dialect.

## Key Files
| File | Description |
|------|-------------|
| `IR/ArmSVEDialect.cpp` | Dialect registration |
| `Transforms/LegalizeForLLVMExport.cpp` | Legalizes scalable vector ops for LLVM IR lowering |
| `Transforms/LegalizeVectorStorage.cpp` | Handles storage of scalable vectors via alloca adjustments |
| `Transforms/LowerContractToSVEPatterns.cpp` | Lowers `vector.contract` to SVE intrinsics |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Dialect definition |
| `Transforms/` | Legalization and contract lowering |
| `TransformOps/` | Transform dialect extension |

## For AI Agents

### Working In This Directory
- Scalable vector types use `vector<[4]xi32>` notation; `vscale` is a runtime value.
- `LegalizeVectorStorage.cpp` must handle that scalable vectors cannot use fixed-size stack slots.
- After legalization, ops lower to `llvm.intr.aarch64.sve.*` intrinsics.
- SVE predicate types (`nxv16i1` etc.) require special handling distinct from fixed-width masks.

### Common Patterns
- Legalization patterns match `vector.transfer_read/write` with scalable types.
- Contract lowering checks for SVE-compatible index maps before rewriting.

## Dependencies
- `mlir/Dialect/Vector`, `mlir/Dialect/LLVMIR`, `mlir/Dialect/MemRef`

<!-- MANUAL: -->
