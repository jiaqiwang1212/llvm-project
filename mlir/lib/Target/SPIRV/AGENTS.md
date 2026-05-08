<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Target/SPIRV

## Purpose
Implements SPIR-V binary serialization and deserialization for the MLIR SPIR-V dialect. Converts between the in-memory SPIR-V dialect ops and the SPIR-V binary word-stream format defined by the Khronos SPIR-V specification.

## Key Files
| File | Description |
|------|-------------|
| `SPIRVBinaryUtils.cpp` | Low-level SPIR-V binary utilities: magic number constants, module header construction, word encoding/decoding helpers |
| `Target.cpp` | `mlir::spirv::serialize()` and `mlir::spirv::deserialize()` top-level entry points |
| `TranslateRegistration.cpp` | Registers `"serialize-spirv"` and `"deserialize-spirv"` with `mlir-translate` |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Serialization/` | Serialization implementation: SPIR-V dialect → binary word stream |
| `Deserialization/` | Deserialization implementation: binary word stream → SPIR-V dialect ops |

## For AI Agents

### Working In This Directory
- SPIR-V opcode tables are generated from the SPIR-V grammar JSON file via TableGen; run the relevant `*IncGen` targets before editing.
- Extensions and capabilities must be declared in the `spirv.module` op for correct serialization.
- The binary format is little-endian 32-bit words; endian handling is in `SPIRVBinaryUtils.cpp`.

### Common Patterns
- `serialize()` takes a `spirv::ModuleOp` and fills a `SmallVector<uint32_t>`.
- `deserialize()` takes a `ArrayRef<uint32_t>` and produces an `OwningOpRef<spirv::ModuleOp>`.

## Dependencies

### Internal
- `mlir/lib/Dialect/SPIRV/` — SPIR-V dialect op definitions

### External
- `llvm/lib/Support` — ADT, endian utilities

<!-- MANUAL: -->
