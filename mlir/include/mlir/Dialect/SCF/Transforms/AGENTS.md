<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCF Transforms

## Purpose
Transformation passes and patterns for the SCF dialect: loop unrolling, tiling, pipelining, bufferization interface implementations, and parallel loop specialization.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |
| `Patterns.h` | Rewrite patterns for SCF ops |
| `TileUsingInterface.h` | Tiling utilities using `TilingInterface` |
| `BufferizableOpInterfaceImpl.h` | Bufferization interface impls for SCF ops |
| `BufferDeallocationOpInterfaceImpl.h` | Buffer deallocation interface impls |

## For AI Agents

### Working In This Directory
- `TileUsingInterface.h` is the main API for tiling ops that implement `TilingInterface`
- Implementations live in `lib/Dialect/SCF/Transforms/`

## Dependencies
- Depends on: SCF IR, Bufferization IR, Tensor/MemRef dialects

<!-- MANUAL: -->
