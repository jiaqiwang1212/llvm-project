<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/mlir-tblgen/

## Purpose
Entry-point header for the `mlir-tblgen` TableGen code generation tool. `MlirTblgenMain()` is the main function that loads `.td` files, runs the selected backend generator, and emits the output.

## Key Files
| File | Description |
|------|-------------|
| `MlirTblgenMain.h` | `MlirTblgenMain()` — main entry point for mlir-tblgen |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `MlirTblgenMain()` is called by the `mlir-tblgen` executable after registering backends.
- Backends are selected via `-gen-<name>` flags and registered via `GenInfo` from `mlir/TableGen/`.
- Custom backends that extend mlir-tblgen call this main after adding their `GenRegistration`.

## Dependencies

### Internal
- `mlir/TableGen/` (GenInfo, all model classes)

### External
- `llvm/TableGen/` (TableGen record parser)
- `llvm/Support/` (CommandLine)

<!-- MANUAL: -->
