<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/SPIRV/

## Purpose
Headers for serializing and deserializing SPIR-V binary format. Provides functions to convert between the MLIR SPIR-V dialect (`spirv.module`) and SPIR-V binary words, plus binary utility helpers.

## Key Files
| File | Description |
|------|-------------|
| `Serialization.h` | `serializeModule()` — serialize MLIR SPIR-V module to binary word stream |
| `Deserialization.h` | `deserializeModule()` — deserialize SPIR-V binary to MLIR SPIR-V dialect |
| `SPIRVBinaryUtils.h` | Utilities for working with SPIR-V binary headers and word encoding |
| `Target.h` | `spirv::TargetEnvAttr` helpers for SPIR-V target environment specification |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Serialization input must be a valid `spirv.module` op; run SPIR-V canonicalization first.
- Deserialization produces a `spirv.module` op which can then be converted to other dialects.
- `SPIRVBinaryUtils.h` provides magic number, header format, and word-encoding constants.

### Common Patterns
- Serialize: `SmallVector<uint32_t> binary; spirv::serialize(spirvModule, binary);`
- Deserialize: `auto module = spirv::deserialize(binary, &context);`

## Dependencies

### Internal
- `mlir/Dialect/SPIRV/` (spirv.module and all SPIR-V ops)
- `mlir/IR/` (MLIRContext)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/ADT/` (SmallVector)

<!-- MANUAL: -->
