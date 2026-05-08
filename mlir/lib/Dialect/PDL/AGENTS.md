<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PDL Dialect

## Purpose
Implements the Pattern Description Language (PDL) dialect — a declarative DSL for expressing MLIR rewrite patterns as MLIR IR. PDL patterns can be compiled (via PDLToPDLInterp) into an efficient bytecode interpreter and applied to IR at runtime.

## Key Files
| File | Description |
|------|-------------|
| `IR/PDL.cpp` | Dialect registration and op implementations |
| `IR/PDLTypes.cpp` | PDL type system: `!pdl.operation`, `!pdl.value`, `!pdl.type`, `!pdl.attribute`, `!pdl.range<T>` |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | PDL ops and types |

## For AI Agents

### Working In This Directory
- PDL patterns use `pdl.pattern`, `pdl.operation`, `pdl.attribute`, `pdl.type`, `pdl.rewrite` ops to describe source/result patterns.
- `pdl.operation` matches an op by name and operand/result/attribute constraints; `pdl.rewrite` describes the replacement.
- PDL is compiled by `PDLToPDLInterp` into `pdl_interp.*` ops which run on a bytecode VM.
- External C++ constraints and rewrites can be registered with the PDL bytecode interpreter via `PDLPatternModule`.
- PDL types (`!pdl.operation`, `!pdl.value`) are first-class values in the PDL program representing matched IR entities.

### Common Patterns
- `pdl.constraint` references externally registered C++ constraint functions.
- `pdl.apply_native_rewrite` calls externally registered C++ rewrite callbacks.
- Patterns specify a `benefit` attribute to control ordering when multiple patterns match.

## Dependencies
- `mlir/IR`, `mlir/Dialect/PDLInterp` (compilation target)

<!-- MANUAL: -->
