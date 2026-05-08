<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenACC Dialect

## Purpose
Implements the OpenACC dialect — MLIR representation of OpenACC parallel programming directives. Models compute constructs (`acc.parallel`, `acc.kernels`, `acc.serial`), data constructs (`acc.copyin`, `acc.copyout`, `acc.create`), loop constructs, and routine directives for GPU offloading via OpenACC.

## Key Files
| File | Description |
|------|-------------|
| `IR/OpenACC.cpp` | Core dialect, op implementations, and verifiers |
| `IR/OpenACCCG.cpp` | Code generation helpers for OpenACC constructs |
| `Transforms/ACCComputeLowering.cpp` | Lowers compute constructs to GPU kernel launches |
| `Transforms/ACCRoutineLowering.cpp` | Lowers `acc.routine` to GPU function attributes |
| `Transforms/ACCLoopTiling.cpp` | Loop tiling transformation for ACC loop constructs |
| `Transforms/ACCImplicitData.cpp` | Infers implicit data clauses |
| `Transforms/ACCSpecializeForDevice.cpp` | Device-specific specialization |
| `Transforms/OffloadTargetVerifier.cpp` | Verifies offload target compatibility |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | OpenACC ops, clause types, code generation helpers |
| `Transforms/` | Compute/data/routine lowering, specialization, implicit data analysis |
| `Analysis/` | Data flow and dependency analysis for ACC constructs |
| `Utils/` | Shared utilities |

## For AI Agents

### Working In This Directory
- OpenACC constructs map to MLIR regions; `acc.parallel` contains a region executed on the device.
- Data clauses (`acc.copyin`, `acc.copyout`) manage host-device data movement; they carry operand attributes for structured/unstructured constructs.
- `ACCImplicitData.cpp` performs analysis to insert missing explicit data clauses (OpenACC implicit data rules).
- The lowering target is `gpu.launch` (via `OpenACCToSCF` and then GPU dialect lowering).
- `acc.routine` marks a function as callable from device code — requires function attribute propagation.

### Common Patterns
- Compute construct lowering: walk the region, identify data operands, generate `gpu.alloc`/`gpu.memcpy` + `gpu.launch`.
- Implicit data analysis: traverse SSA use-def chains to find values crossing host-device boundaries.

## Dependencies
- `mlir/Dialect/GPU`, `mlir/Dialect/SCF`, `mlir/Dialect/MemRef`, `mlir/Dialect/Func`

<!-- MANUAL: -->
