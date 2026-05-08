<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Target/Wasm

## Purpose
Implements import of WebAssembly binary modules into the MLIR WasmSSA dialect. Parses a `.wasm` binary and produces a `wasm_ssa.module` op, enabling MLIR-based analysis and transformation of WebAssembly programs.

## Key Files
| File | Description |
|------|-------------|
| `TranslateFromWasm.cpp` | `mlir::translateWasmToModule()`: uses the wabt library to parse a WebAssembly binary and reconstructs it as MLIR WasmSSA dialect ops |
| `TranslateRegistration.cpp` | Registers `"import-wasm"` with `mlir-translate` |

## For AI Agents

### Working In This Directory
- Requires the wabt (WebAssembly Binary Toolkit) library; only built when `MLIR_ENABLE_WASM` is set.
- The WasmSSA dialect is in `mlir/lib/Dialect/WasmSSA/`.
- Export (WasmSSA → `.wasm`) is not yet implemented in this directory.

## Dependencies

### Internal
- `mlir/lib/Dialect/WasmSSA/` — WasmSSA dialect ops

### External
- wabt library (`libwabt`)
- `llvm/lib/Support` — `llvm::MemoryBuffer`

<!-- MANUAL: -->
