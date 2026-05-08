<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MPIToLLVM

## Purpose
Implements the MPI to LLVM lowering pass. Converts `mpi` dialect operations into LLVM dialect function calls that map to the MPI C library ABI (e.g., `MPI_Send`, `MPI_Recv`, `MPI_Init`).

## Key Files
| File | Description |
|------|-------------|
| `MPIToLLVM.cpp` | Conversion patterns mapping MPI ops to LLVM function call ops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateMPIToLLVMConversionPatterns()`
- `LLVMTypeConverter` handles type lowering; MPI handle types become opaque pointers

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- MPI functions are declared as `llvm.func` external declarations and called via `llvm.call`

## Dependencies
- Headers: `include/mlir/Conversion/MPIToLLVM/`
- Source dialect: `lib/Dialect/MPI/`
- Target dialect: `lib/Dialect/LLVMIR/`

<!-- MANUAL: -->
