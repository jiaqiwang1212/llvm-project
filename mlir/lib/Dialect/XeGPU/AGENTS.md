<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# XeGPU Dialect

## Purpose
Implements the XeGPU dialect — Intel Xe GPU-specific operations for matrix and vector computation. Models subgroup-level matrix ops (`xegpu.dpas` for dot-product accumulate), 2D block load/store, and Intel-specific GPU memory access patterns targeting Intel Graphics Hardware.

## Key Files
| File | Description |
|------|-------------|
| `IR/XeGPUDialect.cpp` | Dialect registration and type definitions |
| `IR/XeGPUOps.cpp` | Op implementations: `xegpu.dpas`, `xegpu.load_nd`, `xegpu.store_nd`, `xegpu.prefetch_nd`, etc. |
| `Transforms/XeGPUBlocking.cpp` | Blocks tensor descriptors into subgroup-sized tiles |
| `Transforms/XeGPULayoutImpl.cpp` | Layout attribute assignment for XeGPU ops |
| `Transforms/XeGPUPropagateLayout.cpp` | Propagates layout annotations through the compute graph |
| `Transforms/XeGPUSubgroupDistribute.cpp` | Distributes workload across subgroup lanes |
| `Transforms/XeGPUUnroll.cpp` | Unrolls XeGPU ops to hardware-sized tiles |
| `Transforms/XeGPUWgToSgDistribute.cpp` | Workgroup to subgroup distribution |
| `Transforms/XeGPUVectorLinearize.cpp` | Linearizes vector layouts for XeVM lowering |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | XeGPU ops, tensor descriptor type, 2D block types |
| `Transforms/` | Blocking, layout propagation, subgroup distribution, unrolling |
| `TransformOps/` | Transform dialect extension for XeGPU |
| `Utils/` | Shared XeGPU utilities |

## For AI Agents

### Working In This Directory
- `xegpu.dpas` is the core instruction — dot product accumulate with subgroup-sized matrix tiles (A×B+C).
- `xegpu.tensor_desc` is XeGPU's typed 2D memory descriptor; `xegpu.load_nd`/`store_nd` use it for 2D block accesses.
- Layout pipeline: `XeGPUBlockingPass` → `XeGPUPropagateLayout` → `XeGPUSubgroupDistribute` before lowering to XeVM.
- `XeGPUWgToSgDistribute` maps workgroup-level tensors to per-subgroup regions.
- Final lowering: `XeGPUToXeVM` converts XeGPU ops to XeVM intrinsic dialect, then `XeVMToLLVM` for LLVM IR.

### Common Patterns
- Tensor descriptor construction: `xegpu.create_nd_tdesc` creates a 2D block descriptor from a memref and base offset.
- Layout attributes: `XeGPULayoutAttr` encodes lane/subgroup distribution; `PropagateLayout` infers them from annotated ops.

## Dependencies
- `mlir/Dialect/GPU`, `mlir/Dialect/Vector`, `mlir/Dialect/MemRef`, `mlir/Dialect/LLVMIR`

<!-- MANUAL: -->
