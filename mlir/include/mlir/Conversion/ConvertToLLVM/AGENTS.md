<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ConvertToLLVM

## Purpose
Provides the generic LLVM IR conversion infrastructure. Declares the umbrella pass and interface for converting arbitrary dialect ops to LLVM dialect via a shared type converter.

## Key Files
| File | Description |
|------|-------------|
| `ToLLVMInterface.h` | C++ interface declarations for LLVM conversion |
| `ToLLVMInterface.td` | TableGen interface definitions (generates ToLLVMInterface.h.inc) |
| `ToLLVMPass.h` | Pass pipeline entry-points and option structs |
| `CMakeLists.txt` | Build rules for this directory |

## For AI Agents

### Working In This Directory
- Edit `.td` files to add or modify interfaces; regenerate with `mlir-tblgen`
- `ToLLVMPass.h` declares the generic `convert-to-llvm` umbrella pass
- The implementation lives under `mlir/lib/Conversion/ConvertToLLVM/`

## Dependencies
- Target dialect: `include/mlir/Dialect/LLVMIR/`
- Shared type converter: `LLVMCommon/`
- Consumed by all `*ToLLVM` passes

<!-- MANUAL: -->
