<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmNeon Dialect

## Purpose
Implements the Arm Neon dialect, providing access to AArch64 NEON SIMD intrinsics that are not expressible via generic Vector ops. Primarily exposes SDOT/UDOT/SMMLA/UMMLA matrix multiply-accumulate instructions for ML workloads.

## Key Files
| File | Description |
|------|-------------|
| `IR/ArmNeonDialect.cpp` | Dialect registration and initialization |
| `Transforms/LowerContractToNeonPatterns.cpp` | Lowers `vector.contract` to NEON intrinsic ops |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Dialect definition |
| `Transforms/` | Pattern lowering from Vector to Neon ops |
| `TransformOps/` | Transform dialect extension for Neon |

## For AI Agents

### Working In This Directory
- Ops map 1:1 to LLVM NEON intrinsics; TableGen in `include/mlir/Dialect/ArmNeon/` defines the ops via `LLVM_IntrOp`.
- `LowerContractToNeonPatterns.cpp` looks for specific `vector.contract` configurations (element types, tile sizes) that map to SDOT/SMMLA patterns.
- After this dialect, ops must be lowered to `llvm.intr.*` via the ArmNeon-to-LLVM conversion before LLVM codegen.

### Common Patterns
- Pattern matching on `vector.contract` indexing maps to detect dot-product and matmul structures.
- Patterns check vector element types and shapes before rewriting to Neon intrinsics.

## Dependencies
- `mlir/Dialect/Vector`, `mlir/Dialect/LLVMIR`, `mlir/Dialect/Arith`

<!-- MANUAL: -->
