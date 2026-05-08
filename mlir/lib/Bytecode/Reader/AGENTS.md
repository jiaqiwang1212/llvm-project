<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Bytecode/Reader

## Purpose
Implements the MLIR bytecode reader: deserializes binary `.mlirbc` files into in-memory IR. Handles format version negotiation, lazy section loading, resource blob attachment, and dispatches to dialect-registered custom decoders for attributes, types, and op properties.

## Key Files
| File | Description |
|------|-------------|
| `BytecodeReader.cpp` | `BytecodeReader` and `mlir::readBytecodeFile()` / `mlir::isBytecodeFile()`: drives section-by-section decoding, reconstructs ops/types/attributes, manages forward references, and handles versioned format upgrades |

## For AI Agents

### Working In This Directory
- The bytecode format version is a constant at the top of `BytecodeReader.cpp`; increment it when the format changes and add backward-compat code for the previous version.
- Dialect-specific type/attribute decoding is dispatched via `BytecodeDialectInterface::readAttribute()` / `readType()`.
- Lazy loading support allows skipping sections until first access; be careful about ordering when adding new sections.

### Common Patterns
- The reader uses a cursor-based `EncodingReader` over a raw memory buffer (no stream seeks).
- Errors use `InFlightDiagnostic` via the `MLIRContext`; the reader returns `LogicalResult`.

## Dependencies

### Internal
- `mlir/lib/IR/` — `MLIRContext`, `Operation`, all type/attr classes
- `mlir/lib/Bytecode/BytecodeOpInterface.cpp`

### External
- `llvm/lib/Support` — `llvm::MemoryBuffer`, ADT

<!-- MANUAL: -->
