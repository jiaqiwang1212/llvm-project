<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/mlir-reduce/

## Purpose
Entry-point header for the `mlir-reduce` test case reduction tool. Provides `MlirReduceMain()` which drives the reduction loop: repeatedly apply reduction passes and test the result with an external oracle until a minimal reproducer is found.

## Key Files
| File | Description |
|------|-------------|
| `MlirReduceMain.h` | `MlirReduceMain()` — main entry point for the mlir-reduce tool |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Call `MlirReduceMain(argc, argv, registry)` after registering dialects and reduction passes.
- The reduction framework and `Tester` are in `mlir/Reducer/`.
- Provide a test script via `--test` that exits 0 for "interesting" (reproduces the bug) inputs.

## Dependencies

### Internal
- `mlir/Reducer/` (reduction passes and Tester)
- `mlir/IR/` (MLIRContext, DialectRegistry)
- `mlir/Pass/` (PassManager)

### External
- `llvm/Support/`

<!-- MANUAL: -->
