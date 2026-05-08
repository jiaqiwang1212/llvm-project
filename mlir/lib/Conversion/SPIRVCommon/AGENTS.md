<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRVCommon

## Purpose
Provides shared utilities and infrastructure used by multiple SPIR-V conversion passes. Contains common pattern helpers, attribute converters, and type utilities reused across `*ToSPIRV` and `SPIRVTo*` conversions.

## Key Files
| File | Description |
|------|-------------|
| `AttrToLLVMConverter.cpp` | Utilities for converting SPIR-V attributes to LLVM equivalents |
| `Pattern.h` | Shared pattern base classes and helpers for SPIR-V conversions |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- This directory is a support library, not a standalone conversion pass
- `Pattern.h` provides base classes like `SPIRVOpLowering<OpTy>` used by other SPIR-V conversion dirs
- `AttrToLLVMConverter.cpp` is used by `SPIRVToLLVM` for attribute translation

### Common Patterns
- Shared helpers reduce boilerplate across `*ToSPIRV` conversion patterns
- Do not add pass registration here — this is a utility library only
- Changes here may affect multiple downstream SPIR-V conversion passes

## Dependencies
- Used by: `lib/Conversion/MemRefToSPIRV/`, `lib/Conversion/SCFToSPIRV/`, `lib/Conversion/SPIRVToLLVM/`, etc.
- Related headers: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
