<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRefToSPIRV

## Purpose
Implements the MemRef to SPIR-V lowering pass. Converts `memref` load, store, and atomic operations into SPIR-V memory access ops, and maps storage classes using `MapMemRefStorageClass`.

## Key Files
| File | Description |
|------|-------------|
| `MemRefToSPIRV.cpp` | Core conversion patterns for memref ops to SPIR-V |
| `MemRefToSPIRVPass.cpp` | Pass registration and pipeline entry point |
| `MapMemRefStorageClassPass.cpp` | Pass to map memref memory spaces to SPIR-V storage classes |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateMemRefToSPIRVPatterns()`
- SPIRVTypeConverter maps `memref` address spaces to SPIR-V storage classes (Workgroup, StorageBuffer, etc.)

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- Storage class mapping must precede type conversion

## Dependencies
- Headers: `include/mlir/Conversion/MemRefToSPIRV/`
- Source dialect: `lib/Dialect/MemRef/`
- Target dialect: `lib/Dialect/SPIRV/`

<!-- MANUAL: -->
