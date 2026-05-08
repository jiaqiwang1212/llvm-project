<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/mlir-opt

## Purpose
Implements `MlirOptMain()`, the entry-point library for the `mlir-opt` tool. Parses command-line pass pipeline specifications, loads the input MLIR file, runs the pipeline, and writes the output. Used directly by `mlir-opt` and by downstream projects that build custom opt-like tools.

## Key Files
| File | Description |
|------|-------------|
| `MlirOptMain.cpp` | `mlir::MlirOptMain()`: sets up `MLIRContext`, parses the pass pipeline from `--pass-pipeline` or individual `--<pass-name>` flags, handles `--split-input-file`, runs the pipeline, prints results or diffs |

## For AI Agents

### Working In This Directory
- `MlirOptMain` is the canonical entry point for custom `opt` tools; downstream projects call it from their own `main.cpp`.
- The `--split-input-file` mode splits on `// -----` and processes each chunk independently — important for test files.
- `--verify-each` (on by default in debug builds) runs the verifier after every pass.

### Common Patterns
- Custom tools call `MlirOptMain(argc, argv, "My Tool", registry)` from their `main()`.
- Tool-specific dialects are added to the `DialectRegistry` before calling `MlirOptMain`.

## Dependencies

### Internal
- `mlir/lib/Pass/` — pass manager
- `mlir/lib/Parser/` — input file parsing
- `mlir/lib/Support/ToolUtilities.cpp` — `splitAndProcessBuffer`

### External
- `llvm/lib/Support` — `llvm::cl`, `llvm::SourceMgr`

<!-- MANUAL: -->
