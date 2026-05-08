<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/Wasm/

## Purpose
Headers for importing WebAssembly binary format into MLIR. Provides a WasmSSA-based importer that converts WebAssembly binary modules into MLIR IR using the WasmSSA dialect, enabling MLIR-based analysis and transformation of WebAssembly programs.

## Key Files
| File | Description |
|------|-------------|
| `WasmBinaryEncoding.h` | WebAssembly binary format constants and encoding definitions |
| `WasmImporter.h` | `WasmImporter` — reads a Wasm binary and produces MLIR WasmSSA dialect ops |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Input is a WebAssembly binary buffer; output is an MLIR module in the WasmSSA dialect.
- `WasmBinaryEncoding.h` contains opcode and section type constants from the Wasm spec.
- The importer is an early-stage feature; not all Wasm features may be supported.

## Dependencies

### Internal
- `mlir/Dialect/` (WasmSSA dialect)
- `mlir/IR/` (MLIRContext, ModuleOp)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/ADT/` (ArrayRef for binary input)

<!-- MANUAL: -->
