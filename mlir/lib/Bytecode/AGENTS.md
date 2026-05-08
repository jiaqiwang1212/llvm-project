<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Bytecode

## Purpose
Implements MLIR's binary bytecode format — a compact, versioned binary encoding of the IR. The bytecode format supports versioned upgrade paths, lazy loading, and resource blobs, and is significantly faster to serialize/deserialize than the textual assembly format.

## Key Files
| File | Description |
|------|-------------|
| `BytecodeOpInterface.cpp` | Implements `BytecodeOpInterface`: per-op hooks for custom property encoding/decoding in the bytecode stream |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Reader/` | Bytecode reader: deserializes binary `.mlirbc` files back to in-memory IR (see `Reader/AGENTS.md`) |
| `Writer/` | Bytecode writer: serializes in-memory IR to binary `.mlirbc` files (see `Writer/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- When the bytecode format version changes, update the version constant in `Reader/BytecodeReader.cpp` and `Writer/BytecodeWriter.cpp` and add a migration path in the reader.
- `BytecodeOpInterface` is how ops opt in to custom property serialization; without it, properties fall back to attribute encoding.
- The format specification is documented in `mlir/docs/BytecodeFormat.md`.

### Common Patterns
- Ops implement `BytecodeOpInterface` by defining `readProperties(BytecodeReader&, OperationState&)` and `writeProperties(BytecodeWriter&)`.
- The reader/writer share an IR numbering scheme (`IRNumbering.h` in `Writer/`).

## Dependencies

### Internal
- `mlir/lib/IR/` — all core IR types
- `mlir/lib/Interfaces/` — `BytecodeOpInterface`

### External
- `llvm/lib/Support` — `llvm::raw_ostream`, `llvm::MemoryBuffer`, ADT

<!-- MANUAL: -->
