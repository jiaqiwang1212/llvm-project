<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Remark

## Purpose
Implements optimization remark streaming for MLIR. Connects the MLIR diagnostic system to LLVM's `RemarkStreamer`, enabling MLIR passes to emit machine-readable optimization remarks (e.g., why a loop was not vectorized) that can be consumed by tools like `opt-viewer` or serialized to YAML/bitstream.

## Key Files
| File | Description |
|------|-------------|
| `RemarkStreamer.cpp` | `mlir::RemarkStreamer`: wraps `llvm::remarks::RemarkStreamer`; installed on the `MLIRContext`; routes MLIR diagnostics tagged as remarks to the LLVM remark output backend |

## For AI Agents

### Working In This Directory
- Remarks are emitted via `mlir::emitRemark(loc, "message")` in pass code.
- The streamer must be installed on the context before passes run; done by `mlir-opt` when `--mlir-pass-remarks` flags are given.
- Output formats (YAML, bitstream) are controlled by the underlying `llvm::remarks::RemarkSerializer`.

### Common Patterns
- Remarks carry an `llvm::remarks::Remark` with pass name, function name, and source location.
- MLIR remarks bridge to LLVM's remark infrastructure using the `FileLineColLoc` from the IR.

## Dependencies

### Internal
- `mlir/lib/IR/Diagnostics.cpp` — diagnostic system
- `mlir/lib/IR/Location.cpp` — `FileLineColLoc`

### External
- `llvm/lib/Remarks` — `llvm::remarks::RemarkStreamer`, serializers
- `llvm/lib/Support` — `llvm::raw_ostream`

<!-- MANUAL: -->
