<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Arith Dialect

## Purpose
Provides integer and floating-point arithmetic operations (add, mul, div, cmp, cast, etc.) that are type-polymorphic over signless integers and IEEE floats. The foundational arithmetic dialect used throughout MLIR.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op and interface definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Passes including type emulation and bufferization (see `Transforms/AGENTS.md`) |
| `Utils/` | Shared arithmetic utilities (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Arith ops are signless: sign semantics live on the op (e.g., `arith.divsi` vs `arith.divui`)
- Integer overflow behavior is specified via `IntegerOverflowFlags` attribute

### Common Patterns
- Op names: `arith.addi`, `arith.mulf`, `arith.cmpi`, `arith.extsi`, etc.
- Implements `ArithFastMathInterface` for floating-point ops

## Dependencies
- No dialect dependencies (foundational)

<!-- MANUAL: -->
