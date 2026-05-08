<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Bytecode/Writer

## Purpose
Implements the MLIR bytecode writer: serializes in-memory IR to the compact binary `.mlirbc` format. Assigns monotonic numeric IDs to all IR entities (ops, types, attributes, strings) via `IRNumbering`, then writes each section in the specified format version.

## Key Files
| File | Description |
|------|-------------|
| `BytecodeWriter.cpp` | `BytecodeWriter` and `mlir::writeBytecodeToFile()`: top-level writer; encodes strings, types, attributes, ops, resources, and properties into sections |
| `IRNumbering.cpp` | `IRNumberingState`: assigns stable integer IDs to all unique IR objects (ops, types, attributes, string literals) used as references within the bytecode |
| `IRNumbering.h` | Internal header declaring `IRNumberingState` and related types |

## For AI Agents

### Working In This Directory
- `IRNumbering` must be run as a pre-pass before writing begins; all IDs are frozen before the first byte is written.
- The writer supports a `BytecodeWriterConfig` for selecting the format version and attaching resource producers.
- Dialect-specific type/attribute encoding dispatches to `BytecodeDialectInterface::writeAttribute()` / `writeType()`.

### Common Patterns
- The writer uses a `EncodingWriter` abstraction over `llvm::raw_ostream` for variable-length integer encoding.
- Resource blobs are written in a separate section and referenced by ID from the main attribute section.

## Dependencies

### Internal
- `mlir/lib/IR/` — core IR traversal
- `mlir/lib/Bytecode/BytecodeOpInterface.cpp`

### External
- `llvm/lib/Support` — `llvm::raw_ostream`, ADT

<!-- MANUAL: -->
