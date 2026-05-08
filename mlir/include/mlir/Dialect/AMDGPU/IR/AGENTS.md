<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AMDGPU IR

## Purpose
Core op, type, attribute, and enum definitions for the AMDGPU dialect. Covers matrix ops (MFMA, WMMA), buffer ops, gather/scatter, and scheduling barrier ops.

## Key Files
| File | Description |
|------|-------------|
| `AMDGPU.td` | Top-level ODS include for all AMDGPU definitions |
| `AMDGPUBase.td` | Dialect definition and shared base classes |
| `AMDGPUOps.td` | Op definitions for all AMDGPU ops |
| `AMDGPUAttrs.td` | Attribute definitions (chipset, etc.) |
| `AMDGPUEnums.td` | Enum definitions |
| `AMDGPUEnums.h` | Generated enum declarations |
| `AMDGPUDialect.h` | Dialect class declaration |
| `AMDGPUTypes.td` | Type definitions |

## For AI Agents

### Working In This Directory
- Edit `AMDGPUOps.td` to add new ops; `AMDGPUBase.td` for shared base classes
- Enums in `.td` generate both C++ enums and LLVM attribute mappings

### Common Patterns
- MFMA ops encode input/output vector types directly in the op name parameters
- Buffer ops follow AMD GCN/CDNA raw buffer semantics

## Dependencies
- Depends on: Vector dialect types, LLVMIR dialect attribute conventions

<!-- MANUAL: -->
