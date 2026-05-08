<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# UB Dialect

## Purpose
Undefined Behavior dialect. Provides ops and interfaces for representing intentional undefined behavior in IR: `ub.poison` for poison values, and the `UBOpInterface` for ops that carry UB semantics.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op and interface definitions (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `ub.poison` creates a poison value of any type (analogous to LLVM `undef`/`poison`)
- Lowered to LLVM `poison` via UBToLLVM conversion
- Op names follow `ub.*` convention

### Common Patterns
- Poison values propagate through arithmetic; their use triggers UB in LLVM semantics

## Dependencies
- Depends on: LLVMIR dialect (for lowering)

<!-- MANUAL: -->
