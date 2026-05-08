<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SparseTensor Dialect

## Purpose
Implements the SparseTensor dialect — MLIR's sparse tensor abstraction and compilation pipeline. Provides sparse tensor types with encoding attributes (CSR, CSC, COO, etc.), ops for sparse tensor construction/conversion, and a full compilation pipeline from sparse linalg ops to dense loops or GPU code.

## Key Files
| File | Description |
|------|-------------|
| `IR/SparseTensorDialect.cpp` | Dialect registration, `SparseTensorEncodingAttr` implementation |
| `IR/SparseTensorInterfaces.cpp` | Interface implementations for sparse ops |
| `Transforms/Sparsification.cpp` | Core sparse code generation from annotated linalg ops |
| `Transforms/SparseTensorCodegen.cpp` | Generates code for sparse tensor storage (data structures) |
| `Transforms/SparseTensorConversion.cpp` | Converts sparse tensor ops to library calls |
| `Transforms/SparseVectorization.cpp` | Vectorizes sparse loops |
| `Transforms/SparseGPUCodegen.cpp` | GPU-specific sparse code generation |
| `Transforms/SparsificationAndBufferizationPass.cpp` | Combined sparsification + bufferization pipeline |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Encoding attribute, sparse tensor ops, interface impls |
| `Transforms/` | Full sparsification pipeline: sparsification, codegen, vectorization, GPU |
| `Pipelines/` | Predefined sparse compilation pipelines |
| `TransformOps/` | Transform dialect extension for sparse transformations |
| `Utils/` | Sparse utility functions (level traversal, coordinate helpers) |

## For AI Agents

### Working In This Directory
- `SparseTensorEncodingAttr` is the key attribute — it specifies the sparse format (levels, ordering, bitwidths).
- Sparsification rewrites `linalg.generic` with sparse tensor operands into iterator-based loops over sparse storage.
- The compilation pipeline: annotate tensors → `Sparsification` → `SparseTensorCodegen` → bufferize → lower to loops.
- `SparseIterationToScf.cpp` converts sparse iteration ops to `scf` loops; this is the bridge from sparse-specific ops to generic SCF.
- GPU sparse codegen (`SparseGPUCodegen.cpp`) targets cuSPARSE and similar libraries via function call generation.
- `IR/Detail/` contains internal type implementation details for sparse encodings.

### Common Patterns
- Encoding attachment: use `RankedTensorType::get(shape, elementType, encodingAttr)` to create a sparse tensor type.
- Sparsification identifies linalg ops with sparse operands and generates specialized loop nests based on the encoding.
- Level coordinates: `SparseTensorLevel` in Utils represents a single level in the sparse storage hierarchy.

## Dependencies
- `mlir/Dialect/Linalg`, `mlir/Dialect/SCF`, `mlir/Dialect/MemRef`, `mlir/Dialect/Bufferization`, `mlir/Dialect/GPU`

<!-- MANUAL: -->
