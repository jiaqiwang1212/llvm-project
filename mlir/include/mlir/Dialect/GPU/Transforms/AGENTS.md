<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPU Transforms

## Purpose
Transformation pass declarations for the GPU dialect: memory promotion, parallel loop mapping, kernel outlining, and bufferization interface implementations.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `MemoryPromotion.h` | Shared memory promotion utilities |
| `ParallelLoopMapper.h` | Parallel loop to GPU thread/block mapping |
| `BufferDeallocationOpInterfaceImpl.h` | Buffer deallocation interface impls for GPU ops |

## For AI Agents

### Working In This Directory
- `MemoryPromotion.h` provides `promoteLoopsToSharedMemory` for GPU shared memory optimization
- Implementations live in `lib/Dialect/GPU/Transforms/`

## Dependencies
- Depends on: GPU IR, MemRef dialect, SCF dialect

<!-- MANUAL: -->
