<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/mlir-reduce

## Purpose
Implements `MlirReduceMain()`, the entry-point library for the `mlir-reduce` IR minimization tool.

## Key Files
| File | Description |
|------|-------------|
| `MlirReduceMain.cpp` | `mlir::MlirReduceMain()`: parses CLI arguments (`--test`, `--test-arg`, reduction strategy flags), loads the input MLIR file, and drives the reduction tree search via `mlir/lib/Reducer/` |

## For AI Agents

### Working In This Directory
- Reduction logic is in `mlir/lib/Reducer/`; this is only the CLI entry point.
- The `--test` script must exit 0 for "interesting" (bug present) and non-zero otherwise.

## Dependencies

### Internal
- `mlir/lib/Reducer/` — reduction engine
- `mlir/lib/Parser/` — input parsing

### External
- `llvm/lib/Support` — `llvm::cl`, subprocess execution

<!-- MANUAL: -->
