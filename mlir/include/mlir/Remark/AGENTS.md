<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Remark/

## Purpose
Optimization remark streaming infrastructure for MLIR. Provides `RemarkStreamer`, which bridges MLIR's diagnostic system to LLVM's `remarks::RemarkStreamer` for emitting optimization remarks in YAML or bitstream format. Used by passes to report transformation decisions (e.g., inlining succeeded, vectorization failed).

## Key Files
| File | Description |
|------|-------------|
| `RemarkStreamer.h` | `RemarkStreamer` — wraps LLVM's remark emitter, connects to MLIR diagnostic handlers |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `RemarkStreamer` is attached to `MLIRContext` and activated by `--mlir-pass-remarks` style options.
- Passes emit remarks via the `PassManager`'s instrumentation layer, not directly through this header.
- Output formats: YAML (`-pass-remarks-output`) or bitstream (`-pass-remarks-format=bitstream`).

### Common Patterns
- Setup: `context.enableOptimizationRemarks(streamer)`.
- Remark emission is typically handled transparently by the pass manager instrumentation.

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext, Location)
- `mlir/Pass/` (PassInstrumentation)

### External
- `llvm/Remarks/` (LLVM remark streamer and serializers)

<!-- MANUAL: -->
