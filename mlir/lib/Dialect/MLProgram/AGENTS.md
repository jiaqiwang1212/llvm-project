<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MLProgram Dialect

## Purpose
Implements the MLProgram dialect — a high-level ML program representation targeting inference and training frameworks. Provides global variables (`ml_program.global`), token-based ordering (`ml_program.token`), and subgraph ops for structured ML computation graphs.

## Key Files
| File | Description |
|------|-------------|
| `IR/MLProgramDialect.cpp` | Dialect registration |
| `IR/MLProgramOps.cpp` | Op implementations: `ml_program.global`, `ml_program.global_load`, `ml_program.global_store`, `ml_program.output`, `ml_program.return` |
| `Transforms/BufferizableOpInterfaceImpl.cpp` | Bufferization support for MLProgram tensor ops |
| `Transforms/PipelineGlobalOps.cpp` | Pipeline pass for global variable access optimization |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Global variable and program structure ops |
| `Transforms/` | Bufferization and global pipelining |

## For AI Agents

### Working In This Directory
- `ml_program.global` holds module-level state (weights, buffers) accessed via load/store ops with token ordering.
- Token-based ordering (`ml_program.token`) provides explicit data-flow edges for mutation ordering without aliasing analysis.
- The dialect is used as a target for TOSA/StableHLO frontend lowering before further lowering to Linalg/MemRef.
- `PipelineGlobalOps.cpp` hoists global loads out of loops when the global is not mutated in the loop body.

### Common Patterns
- Global declarations: `MLProgramGlobalOp` with a type attribute and optional initial value.
- Token threading: `ml_program.global_store` produces a token consumed by subsequent loads to enforce ordering.

## Dependencies
- `mlir/Dialect/Tensor`, `mlir/Dialect/Func`

<!-- MANUAL: -->
