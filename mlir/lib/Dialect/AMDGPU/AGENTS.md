<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AMDGPU Dialect

## Purpose
Implements AMD GPU-specific operations that map to CDNA/RDNA ISA features not expressible in the generic GPU dialect: matrix intrinsics (MFMA, WMMA), raw buffer operations, lds_barrier, gather/scatter, and AMD-specific memory semantics.

## Key Files
| File | Description |
|------|-------------|
| `IR/AMDGPUDialect.cpp` | Dialect registration and initialization |
| `IR/AMDGPUOps.cpp` | Op implementations and verifiers |
| `IR/AMDGPUAttrs.cpp` | Attribute definitions (chipset versions, matrix types) |
| `IR/AMDGPUTypes.cpp` | Type definitions for AMD-specific types |
| `IR/AMDGPUEnums.cpp` | Enum definitions for AMD hardware variants |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core IR: ops, attributes, types, enums |
| `Transforms/` | AMD-specific optimization passes |
| `Utils/` | Shared utilities |

## For AI Agents

### Working In This Directory
- Ops target specific GPU chipsets (gfx9xx, gfx10xx, gfx11xx); verifiers check chipset compatibility via `AMDGPUAttrs`.
- MFMA/WMMA ops have strict constraints on element types and tile sizes tied to hardware; check AMD ISA docs.
- `EmulateAtomics.cpp` handles lowering of atomic ops for chipsets that lack native support.
- `FoldMemRefsOps.cpp` and `ResolveStridedMetadata.cpp` assist in lowering memref ops to raw buffer ops.

### Common Patterns
- Op verifiers check chipset attribute for feature availability.
- Matrix ops use `AMDGPUMatrixCoreIntrinsic` enum to select the right intrinsic.
- Transforms interact with `ROCDLDialect` for final lowering.

## Dependencies
- `mlir/Dialect/GPU`, `mlir/Dialect/LLVMIR`, `mlir/Dialect/Vector`, `mlir/Dialect/MemRef`

<!-- MANUAL: -->
