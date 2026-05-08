<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRVToLLVM

## Purpose
Declares the SPIR-V to LLVM IR lowering pass. Lowers SPIR-V dialect ops to LLVM dialect ops, enabling host-side execution or further LLVM backend compilation of SPIR-V modules.

## Key Files
| File | Description |
|------|-------------|
| `SPIRVToLLVM.h` | Conversion pattern declarations |
| `SPIRVToLLVMPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `SPIRVToLLVMPass.h` for pass registration; edit `SPIRVToLLVM.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/SPIRVToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/SPIRV/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`
- Shared utilities: `SPIRVCommon/`, `LLVMCommon/`

<!-- MANUAL: -->
