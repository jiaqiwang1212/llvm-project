<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# WasmSSA Dialect

## Purpose
Implements the WasmSSA dialect — an MLIR representation of WebAssembly in SSA form. Models WebAssembly structured control flow (`wasmssa.block`, `wasmssa.loop`, `wasmssa.if`), memory ops, and the WebAssembly type system (`wasmssa.funcref`, `wasmssa.externref`) for compiling to or analyzing WebAssembly targets.

## Key Files
| File | Description |
|------|-------------|
| `IR/WasmSSADialect.cpp` | Dialect registration |
| `IR/WasmSSAOps.cpp` | Op implementations for WebAssembly structured control flow and memory ops |
| `IR/WasmSSAInterfaces.cpp` | Interface implementations |
| `IR/WasmSSATypes.cpp` | WebAssembly reference types: `funcref`, `externref` |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | WebAssembly ops, reference types, interfaces |

## For AI Agents

### Working In This Directory
- WasmSSA models WebAssembly's structured control flow (not arbitrary CFG) — blocks, loops, and if-else are first-class ops with regions.
- Reference types (`funcref`, `externref`) are WebAssembly-specific; they cannot be directly lowered to scalar integer types.
- No transforms are defined here — this dialect is primarily a representation target for WebAssembly frontends or analysis tools.

### Common Patterns
- Structured control flow: `wasmssa.block`/`wasmssa.loop` regions terminate with `wasmssa.br`/`wasmssa.br_if`.
- Type constraints: ops verify that reference type operands match the Wasm type system rules.

## Dependencies
- `mlir/IR`, `mlir/Dialect/ControlFlow`

<!-- MANUAL: -->
