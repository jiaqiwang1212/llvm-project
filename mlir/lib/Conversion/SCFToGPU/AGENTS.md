<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToGPU

## Purpose
Implements the SCF to GPU lowering pass. Converts `scf.parallel` ops (and optionally `scf.for` loops with annotations) into `gpu.launch` blocks with thread/block index mappings for GPU execution.

## Key Files
| File | Description |
|------|-------------|
| `SCFToGPU.cpp` | Conversion patterns mapping parallel loops to gpu.launch regions |
| `SCFToGPUPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateParallelLoopToGPUPatterns()`
- Loop induction variables map to `gpu.block_id` / `gpu.thread_id` index computations

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- The outermost parallel loop dimensions map to grid (block) dimensions
- Inner loop dimensions map to block (thread) dimensions
- Loop bounds become the `gridDim`/`blockDim` arguments of `gpu.launch`

## Dependencies
- Headers: `include/mlir/Conversion/SCFToGPU/`
- Source dialect: `lib/Dialect/SCF/`
- Target dialect: `lib/Dialect/GPU/`

<!-- MANUAL: -->
