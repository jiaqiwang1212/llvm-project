<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCopy/wasm

## Purpose

Implements object file transformation for WebAssembly (WASM) modules. Handles reading, modifying, and writing WASM module structure including sections, code, data, and custom metadata.

## Key Files

| File | Description |
|------|-------------|
| `WasmObjcopy.cpp` | Main WASM module transformation logic |
| `WasmObject.cpp` | WASM module representation and modification |
| `WasmObject.h` | WASM object interface definition |
| `WasmReader.cpp` | WASM module file parsing |
| `WasmReader.h` | WASM reader interface |
| `WasmWriter.cpp` | WASM module serialization |
| `WasmWriter.h` | WASM writer interface |

## For AI Agents

### Working In This Directory

1. Understand WASM binary format: module structure, sections (code, data, imports, exports, etc.)
2. Know WASM instruction set and validation rules
3. Test with real WASM modules (.wasm files)
4. Verify section ordering, element offsets, and function indices
5. Handle custom sections for metadata and debugging
6. Ensure relocation updates maintain function/data references
7. Test with WASM runtimes (Node.js, browser, wasmtime, etc.)

### Key Patterns

- WASM module organized as typed sections in specific order
- Code section contains function bodies; data section contains initialized globals
- Imports/exports define module interface
- Custom sections carry debug info (name section, .debug_info, etc.)
- Section mutations must update element counts and index references

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM Object, LLVM BinaryFormat, ObjCopy common infrastructure
- Used by: llvm-objcopy when processing WebAssembly modules

<!-- MANUAL: -->
