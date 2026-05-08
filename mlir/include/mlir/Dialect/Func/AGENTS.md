<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Func Dialect

## Purpose
Provides function definition and call ops (`func.func`, `func.call`, `func.return`, `func.constant`) for representing callable units in MLIR IR. The standard dialect for function-level structure.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Extensions/` | Inliner and sharding extensions (see `Extensions/AGENTS.md`) |
| `IR/` | Op definitions (see `IR/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | Function utility helpers (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `func.func` is the canonical function op; it has a `FunctionType` and a single-block or multi-block body
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- Op names: `func.func`, `func.call`, `func.call_indirect`, `func.return`, `func.constant`
- Functions carry symbol names and visibility attributes

## Dependencies
- No dialect dependencies (foundational)

<!-- MANUAL: -->
