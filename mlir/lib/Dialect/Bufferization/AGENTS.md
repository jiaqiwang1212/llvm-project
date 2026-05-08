<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bufferization Dialect

## Purpose
Implements One-Shot Bufferization — MLIR's primary tensor-to-memref conversion framework. Analyzes tensor ops to determine aliasing and ownership, inserts buffer copies only where necessary, and manages buffer deallocation.

## Key Files
| File | Description |
|------|-------------|
| `IR/BufferizableOpInterface.cpp` | Core interface that ops implement to participate in bufferization |
| `IR/BufferizationDialect.cpp` | Dialect registration |
| `IR/BufferizationOps.cpp` | `bufferization.alloc_tensor`, `bufferization.to_tensor`, `bufferization.to_memref` |
| `Transforms/OneShotAnalysis.cpp` | Alias and ownership analysis for One-Shot Bufferization |
| `Transforms/OneShotModuleBufferize.cpp` | Module-level bufferization entry point |
| `Transforms/OwnershipBasedBufferDeallocation.cpp` | Inserts dealloc ops based on ownership analysis |
| `Transforms/Bufferize.cpp` | Core bufferization rewrite driver |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core interfaces, dialect, and fundamental ops |
| `Transforms/` | Bufferization analysis, rewriting, deallocation, optimization |
| `Extensions/` | External model implementations for ops in other dialects |
| `Pipelines/` | Predefined bufferization pipeline compositions |
| `TransformOps/` | Transform dialect extension for bufferization |

## For AI Agents

### Working In This Directory
- To bufferize a new op, implement `BufferizableOpInterface` — specifically `bufferize()`, `getAliasingValues()`, and `bufferizesToMemoryWrite()`.
- One-Shot Analysis builds an alias set graph; modifying it requires understanding `AnalysisState` and `OneShotAnalysisState`.
- `alloc_tensor` is the canonical way to introduce a new buffer during bufferization.
- Deallocation uses ownership tokens — do not manually insert `memref.dealloc`; let the ownership pass handle it.
- Extensions/ contains `BufferizableOpInterface` impls for ops defined outside this dialect (e.g., func, arith).

### Common Patterns
- `bufferize()` method: call `rewriter.replaceOpWithNewOp<memref::SomeOp>(...)` after resolving buffer aliases.
- `getAliasingValues()`: return the set of result/operand pairs that alias.
- Dealloc insertion: `OwnershipBasedBufferDeallocation` traverses the CFG inserting `memref.dealloc` at dominance frontiers.

## Dependencies
- `mlir/Dialect/MemRef`, `mlir/Dialect/Tensor`, `mlir/Dialect/Func`, `mlir/Analysis/DataFlow`

<!-- MANUAL: -->
