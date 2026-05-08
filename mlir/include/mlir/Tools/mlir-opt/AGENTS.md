<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/mlir-opt/

## Purpose
Entry-point header for `mlir-opt`-style optimization driver tools. `MlirOptMain()` is the standard main function used by all MLIR optimization driver executables. It handles argument parsing, dialect/pass registration, input parsing, pipeline execution, and output.

## Key Files
| File | Description |
|------|-------------|
| `MlirOptMain.h` | `MlirOptMain()` — main entry point; `MlirOptMainConfig` for programmatic configuration |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Call `MlirOptMain(argc, argv, "tool-name", registry)` after registering all dialects and passes.
- `MlirOptMainConfig` allows programmatic control of options (e.g., disabling threading for tests).
- Supports `// -----` split-file testing via `--split-input-file` flag (uses `ParseUtilities.h`).
- `--mlir-print-ir-before-all` / `--mlir-print-ir-after-all` flags are provided automatically.

### Common Patterns
- Minimal tool: `int main(int argc, char **argv) { DialectRegistry reg; registerAllDialects(reg); return mlir::MlirOptMain(argc, argv, "my-opt", reg); }`

## Dependencies

### Internal
- `mlir/IR/` (DialectRegistry, MLIRContext)
- `mlir/Pass/` (PassManager, PassRegistry)
- `mlir/Tools/ParseUtilities.h`

### External
- `llvm/Support/CommandLine.h`

<!-- MANUAL: -->
