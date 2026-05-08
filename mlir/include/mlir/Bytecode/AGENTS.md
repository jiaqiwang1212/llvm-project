<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Bytecode/

## Purpose
Bytecode serialization and deserialization headers for MLIR. Defines the reader and writer interfaces for MLIR's binary bytecode format, which provides faster parsing and smaller file sizes compared to text format. Also contains the dialect interface for customizing bytecode encoding and the op interface for ops with custom bytecode representations.

## Key Files
| File | Description |
|------|-------------|
| `BytecodeReader.h` | `readBytecodeFile()` / `isBytecodeFile()` — deserialize bytecode to IR |
| `BytecodeReaderConfig.h` | `BytecodeReaderConfig` — callbacks and dialect version handling for reading |
| `BytecodeWriter.h` | `writeBytecodeToFile()` — serialize IR to bytecode |
| `BytecodeImplementation.h` | `DialectBytecodeReader`/`Writer` interfaces for custom dialect encoding |
| `BytecodeOpInterface.h` | C++ header for `BytecodeOpInterface` (generated from `.td`) |
| `BytecodeOpInterface.td` | TableGen definition of `BytecodeOpInterface` for ops with custom encoding |
| `BytecodeDialectInterface.td` | TableGen base for dialect-level bytecode hooks |
| `Encoding.h` | Bytecode format constants and encoding utilities |
| `CMakeLists.txt` | Build rules for bytecode tablegen targets |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Dialects customize bytecode encoding by implementing `BytecodeDialectInterface`.
- Individual ops can override encoding via `BytecodeOpInterface`.
- Versioning: `BytecodeReaderConfig` supports dialect version callbacks for forward/backward compatibility.
- `isBytecodeFile()` checks for the bytecode magic bytes before attempting to read.

### Common Patterns
- Write: `writeBytecodeToFile(module, outputStream, config)`
- Read: `readBytecodeFile(inputBuffer, block, config)`
- Dialect interface registration: `addInterfaces<MyDialectBytecodeInterface>()` in dialect constructor.

## Dependencies

### Internal
- `mlir/IR/` (Operation, MLIRContext, Dialect)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/` (raw_ostream, MemoryBuffer)

<!-- MANUAL: -->
