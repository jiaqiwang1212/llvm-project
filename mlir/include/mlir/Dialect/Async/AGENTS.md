<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Async Dialect

## Purpose
Represents asynchronous execution: async functions, tokens, values, and groups for expressing task-parallel computations. Provides the basis for lowering async regions to runtime calls or coroutines.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, and dialect definitions (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files
- Async tokens and values are first-class types

### Common Patterns
- Op names: `async.execute`, `async.await`, `async.yield`, etc.
- Lowered to LLVM coroutines or runtime calls via AsyncToLLVM conversion

## Dependencies
- Depends on: Func dialect, LLVMIR dialect (for lowering)

<!-- MANUAL: -->
