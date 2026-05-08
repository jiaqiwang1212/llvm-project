<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# UB Dialect

## Purpose
Implements the UB (Undefined Behavior) dialect — provides the `ub.poison` op that produces a poison value of any type. Poison values model undefined behavior explicitly in the IR, enabling optimizations to propagate and detect UB without special-casing every op.

## Key Files
| File | Description |
|------|-------------|
| `IR/UBOps.cpp` | `ub.poison` op implementation |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Poison op |

## For AI Agents

### Working In This Directory
- `ub.poison` produces a value of any type that represents undefined behavior — similar to LLVM's `undef`/`poison`.
- Consumers of poison values may produce poison themselves (poison propagation semantics).
- Lowering: `UBToLLVM` lowers `ub.poison` to `llvm.mlir.undef`; `UBToSPIRV` lowers to `spirv.Undef`.
- This dialect is intentionally minimal — only the `ub.poison` op exists.
- Canonicalizers in other dialects use `ub.poison` to represent results of ops with UB inputs (e.g., `arith.divsi` by zero).

### Common Patterns
- Pattern: when folding an op with known-UB inputs, return `ub.poison` for the result.
- Type constraint: `ub.poison` is typed (`ub.poison : i32`, `ub.poison : tensor<4xf32>`) — it matches the result type needed.

## Dependencies
- `mlir/IR`, `mlir/Dialect/LLVMIR` (for lowering), `mlir/Dialect/SPIRV` (for lowering)

<!-- MANUAL: -->
