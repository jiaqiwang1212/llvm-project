<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCF Dialect

## Purpose
Implements the Structured Control Flow (SCF) dialect — MLIR's high-level structured loop and conditional constructs: `scf.for`, `scf.while`, `scf.if`, `scf.parallel`, `scf.forall`, `scf.index_switch`. Sits between high-level parallel abstractions and the `cf` (unstructured control flow) dialect.

## Key Files
| File | Description |
|------|-------------|
| `IR/SCF.cpp` | All SCF op implementations: `scf.for`, `scf.while`, `scf.if`, `scf.parallel`, `scf.forall`, `scf.reduce` |
| `IR/DeviceMappingInterface.cpp` | Device mapping interface for `scf.forall` (GPU block/thread mapping) |
| `IR/MemorySlot.cpp` | Memory slot analysis for SCF ops |
| `IR/ValueBoundsOpInterfaceImpl.cpp` | Value bounds for loop induction variables |
| `Transforms/TileUsingInterface.cpp` | Tiles ops implementing `TilingInterface` into `scf.for`/`scf.forall` loops |
| `Transforms/LoopPipelining.cpp` | Software pipelining of `scf.for` loops |
| `Transforms/ForallToFor.cpp` | Lowers `scf.forall` to `scf.for` |
| `Transforms/LoopSpecialization.cpp` | Specializes loops for boundary conditions |
| `Transforms/ParallelLoopTiling.cpp` | Tiles `scf.parallel` loops |
| `Transforms/StructuralTypeConversions.cpp` | Type conversion for loop-carried values |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Loop and conditional ops, device mapping, value bounds |
| `Transforms/` | Tiling, pipelining, specialization, parallel loop transformations |
| `TransformOps/` | Transform dialect extension for SCF |
| `Utils/` | Loop utility functions |

## For AI Agents

### Working In This Directory
- `scf.for` carries loop-carried values as iter_args; `scf.yield` returns updated values at end of each iteration.
- `scf.forall` is the parallel-semantics loop; it uses `scf.forall.in_parallel` terminator and supports device mapping attributes.
- `scf.parallel` is the older parallel loop with explicit reduction ops; prefer `scf.forall` for new code.
- `TileUsingInterface` is the generic tiling engine — it works on any op implementing `TilingInterface`, not just linalg ops.
- `LoopPipelining` applies software pipelining with a user-specified stage assignment annotation.
- `StructuralTypeConversions.cpp` handles updating iter_arg types during dialect conversion passes.

### Common Patterns
- Loop bound inference: `ValueBoundsOpInterfaceImpl` provides `scf.for`'s IV bounds for integer range analysis.
- Device mapping: attach `DeviceMappingAttrInterface` to `scf.forall` to map iterations to GPU blocks/threads.
- Tiling: pass a `TilingInterface` op and tile sizes to `tileUsingSCF()`.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/Dialect/MemRef`, `mlir/Dialect/Tensor`, `mlir/Dialect/ControlFlow`

<!-- MANUAL: -->
