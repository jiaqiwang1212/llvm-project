<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/IR/PDL

## Purpose
Implements PDL (Pattern Description Language) pattern matching support within the core IR library. Provides the runtime match/rewrite glue used when PDL-defined patterns interact with the core `PatternMatch` infrastructure.

## Key Files
| File | Description |
|------|-------------|
| `PDLPatternMatch.cpp` | Implements `PDLPatternModule` and the binding between compiled PDL bytecode patterns and the `RewritePattern` / `PatternRewriter` APIs |

## For AI Agents

### Working In This Directory
- This directory is a thin bridge; the heavy lifting is in `mlir/lib/Rewrite/ByteCode.cpp` (PDL bytecode interpreter) and the PDL dialect under `mlir/lib/Dialect/PDL/`.
- Changes here usually accompany changes to `mlir/include/mlir/IR/PatternMatch.h`.

### Common Patterns
- PDL patterns are compiled to bytecode by `mlir/lib/Rewrite/`; this directory handles the `PatternRewriter` callback interface that the bytecode interpreter calls back into.

## Dependencies

### Internal
- `mlir/lib/IR/PatternMatch.cpp` — base `RewritePattern` and `PatternRewriter`
- `mlir/lib/Rewrite/` — PDL bytecode interpreter
- `mlir/lib/Dialect/PDL/` — PDL dialect ops

### External
- `llvm/lib/Support` — ADT utilities

<!-- MANUAL: -->
