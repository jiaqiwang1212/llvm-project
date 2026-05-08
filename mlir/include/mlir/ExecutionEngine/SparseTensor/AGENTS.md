<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/ExecutionEngine/SparseTensor/

## Purpose
Sparse tensor storage format headers used by the sparse tensor runtime library. Defines the coordinate-order (COO) format, compressed storage formats, file I/O for sparse tensors, and arithmetic utilities used by the lowered code from the SparseTensor dialect.

## Key Files
| File | Description |
|------|-------------|
| `ArithmeticUtils.h` | Arithmetic helper utilities for sparse tensor computations |
| `COO.h` | Coordinate-order (COO) sparse tensor storage format |
| `File.h` | Sparse tensor file I/O (Matrix Market, FROSTT formats) |
| `MapRef.h` | `MapRef` — a reference-counted map for sparse tensor dimension ordering |
| `Storage.h` | Compressed sparse storage formats (CSR, CSC, etc.) and tensor metadata |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- These headers define the runtime ABI between lowered sparse tensor ops and the runtime library.
- They are compiled into both the runtime shared library and test executables.
- `Storage.h` defines `SparseTensorStorage<P,C,V>` — the primary runtime sparse tensor type.
- File formats supported: Matrix Market (`.mtx`), FROSTT (`.tns`).
- Changes here affect ABI between compiled MLIR code and the runtime — be careful with struct layouts.

### Common Patterns
- Runtime usage: `SparseTensorStorage<uint64_t, uint64_t, double>` for a CSR-like tensor.
- COO construction: used during file loading before converting to compressed format.

## Dependencies

### Internal
- `mlir/ExecutionEngine/SparseTensorRuntime.h` (runtime function declarations)

### External
- Standard C++ library (`<vector>`, `<algorithm>`, etc.)

<!-- MANUAL: -->
