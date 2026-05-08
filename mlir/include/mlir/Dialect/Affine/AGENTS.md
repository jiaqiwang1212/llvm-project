<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Affine Dialect

## Purpose
Represents affine loop nests and memory accesses using polyhedral abstractions. Provides affine maps, integer sets, and structured control flow (for/if) for high-performance loop transformations and analysis.

## Key Files
| File | Description |
|------|-------------|
| `LoopFusionUtils.h` | Utilities for fusing affine loops |
| `LoopUtils.h` | Loop transformation utilities (tiling, unrolling, pipelining) |
| `Utils.h` | General affine dialect utilities |
| `ViewLikeInterfaceUtils.h` | Utilities for view-like op interface implementations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Analysis/` | Affine analysis passes (see `Analysis/AGENTS.md`) |
| `IR/` | Op, interface, and value-map definitions (see `IR/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops for affine (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- ODS `.td` files in `IR/` generate C++ — edit `.td`, not generated `.h.inc` files
- Affine ops are constrained to affine.for/if regions; verify affine-map restrictions when adding ops

### Common Patterns
- Affine maps and integer sets are the core abstractions; ops carry `AffineMap` attributes
- Implements `LoopLikeOpInterface` and `AffineMapAccessInterface`

## Dependencies
- Depends on: MemRef dialect (for affine loads/stores), Arith dialect (for loop bounds)

<!-- MANUAL: -->
