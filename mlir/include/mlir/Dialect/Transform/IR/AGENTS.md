<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform IR

## Purpose
Core op, type, attribute, and dialect definitions for the Transform dialect: sequences, named sequences, foreach, apply patterns, merge handles, and the primary handle/param types.

## Key Files
| File | Description |
|------|-------------|
| `TransformDialect.h` | Dialect class declaration |
| `TransformDialect.td` | Dialect definition |
| `TransformOps.h` | Op class declarations |
| `TransformOps.td` | ODS op definitions (sequence, foreach, apply, etc.) |
| `TransformTypes.h` | Type class declarations |
| `TransformTypes.td` | ODS type definitions (any_op, op<>, param<>, value handle) |
| `TransformAttrs.h` | Attribute class declarations |
| `TransformAttrs.td` | ODS attribute definitions |
| `Utils.h` | Core utility functions used by transform ops |

## For AI Agents

### Working In This Directory
- `transform.named_sequence` defines reusable transform functions callable with `transform.include`
- `transform.foreach` iterates over a handle's payload ops one-by-one
- `transform.apply_patterns` applies a set of rewrite patterns to the payload

## Dependencies
- Depends on: Transform Interfaces, MLIR IR core

<!-- MANUAL: -->
