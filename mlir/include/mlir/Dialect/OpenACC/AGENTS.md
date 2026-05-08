<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenACC Dialect

## Purpose
Represents OpenACC directive-based GPU offloading: compute constructs (parallel, kernels, serial), data clauses, loop constructs, and atomic ops. Provides a high-level representation for OpenACC programs before lowering to GPU/OpenMP backends.

## Key Files
| File | Description |
|------|-------------|
| `OpenACC.h` | Op class declarations |
| `OpenACCBase.td` | Dialect definition and base classes |
| `OpenACCOps.td` | ODS op definitions |
| `OpenACCAttributes.td` | Attribute definitions |
| `OpenACCCGOps.td` | Code-generation op definitions |
| `OpenACCCGAttributes.td` | Code-generation attribute definitions |
| `OpenACCOpsInterfaces.td` | Op interface definitions |
| `OpenACCOpsTypes.td` | Type definitions |
| `OpenACCTypeInterfaces.td` | Type interface definitions |
| `OpenACCParMapping.h` | Parallel mapping attribute helpers |
| `OpenACCUtils.h` | General OpenACC utilities |
| `OpenACCUtilsCG.h` | Code-generation utilities |
| `OpenACCUtilsGPU.h` | GPU-specific utilities |
| `OpenACCUtilsLoop.h` | Loop clause utilities |
| `OpenACCUtilsTiling.h` | Tiling clause utilities |
| `OpenACCVariableInfo.h` | Variable info for data clause analysis |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Analysis/` | OpenACC support analysis (see `Analysis/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- OpenACC ops carry clause operands as named variadic operand groups
- Lowered to SCF/GPU via OpenACCToSCF or to LLVM via GPUToLLVM paths

### Common Patterns
- `acc.parallel`, `acc.kernels`, `acc.serial` are the top-level compute constructs
- Data clauses (`acc.copyin`, `acc.copyout`, `acc.create`) manage host-device transfers

## Dependencies
- Depends on: MemRef dialect, SCF dialect, GPU dialect (for lowering)

<!-- MANUAL: -->
