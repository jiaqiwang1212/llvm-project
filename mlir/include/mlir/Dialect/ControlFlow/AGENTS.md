<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlow Dialect

## Purpose
Provides low-level control flow ops: conditional branch (`cf.cond_br`), unconditional branch (`cf.br`), assert, and switch. Represents CFG-level control flow after structured control flow is lowered.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Bufferization and structural type conversion helpers (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- These are block-terminator ops for explicit CFG regions
- Op names follow `cf.*` convention

### Common Patterns
- `cf.cond_br` is the primary conditional branch with true/false block args
- Lowered from SCF (SCFToControlFlow) or used directly in LLVM-level IR

## Dependencies
- No dialect dependencies (foundational CFG)

<!-- MANUAL: -->
