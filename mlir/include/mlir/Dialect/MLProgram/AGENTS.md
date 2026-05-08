<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MLProgram Dialect

## Purpose
Represents machine learning program structure: global variables, functions, and token-ordered execution for ML frameworks (e.g., TensorFlow/JAX program representation). Provides a higher-level program model for ML compilation.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, and attribute definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `ml_program.global` holds module-level tensor state
- `ml_program.func` and `ml_program.return` define ML program functions
- Op names follow `ml_program.*` convention

### Common Patterns
- Globals are read/written with `ml_program.global_load` / `ml_program.global_store`
- Token ordering (`ml_program.token`) enforces side-effect ordering

## Dependencies
- Depends on: Func dialect, Tensor dialect

<!-- MANUAL: -->
