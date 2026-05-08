<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenMP Dialect

## Purpose
Represents OpenMP directive-based shared-memory parallel programming: parallel regions, worksharing constructs (for, sections, single), SIMD, task, target offload, atomic ops, and reduction clauses.

## Key Files
| File | Description |
|------|-------------|
| `OpenMPDialect.h` | Dialect class declaration |
| `OpenMPDialect.td` | Dialect definition |
| `OpenMPOps.td` | ODS op definitions |
| `OpenMPAttrDefs.td` | Attribute definitions |
| `OpenMPClauses.td` | Clause mixin definitions |
| `OpenMPClauseOperands.h` | Clause operand struct declarations |
| `OpenMPEnums.td` | Enum definitions |
| `OpenMPInterfaces.h` | Interface declarations |
| `OpenMPOpsInterfaces.td` | ODS op interface definitions |
| `OpenMPTypeInterfaces.td` | ODS type interface definitions |
| `OpenMPOpsAttributes.h` | Attribute class declarations |
| `OpenMPOpsEnums.h` | Generated enum declarations |
| `OpenMPOffloadUtils.h` | Target offload utility functions |
| `OpenMPOpBase.td` | Base classes for OpenMP ops |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- OpenMP ops use clause mixins from `OpenMPClauses.td` to share clause operand structure
- Target offload ops (`omp.target`, `omp.target_data`) bridge CPU and GPU code

### Common Patterns
- `omp.parallel`, `omp.wsloop`, `omp.simd`, `omp.task` are the primary construct ops
- Reductions use `omp.declare_reduction` + per-construct reduction clause operands

## Dependencies
- Depends on: LLVMIR dialect (for lowering), MemRef dialect, Func dialect

<!-- MANUAL: -->
