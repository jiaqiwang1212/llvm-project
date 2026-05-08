<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform Dialect

## Purpose
Implements the Transform dialect — MLIR's programmable compiler transformation framework. Transform ops describe compiler transformations (tile, fuse, vectorize, etc.) as first-class MLIR ops that are interpreted at compile time to restructure the payload IR. Enables user-controlled compilation pipelines via transform scripts.

## Key Files
| File | Description |
|------|-------------|
| `IR/TransformDialect.cpp` | Dialect registration and extension mechanism |
| `IR/TransformOps.cpp` | Core transform ops: `transform.sequence`, `transform.apply_patterns`, `transform.get_parent_op`, etc. |
| `IR/TransformTypes.cpp` | Handle types: `!transform.op<"...">`, `!transform.any_op`, `!transform.any_value` |
| `IR/TransformAttrs.cpp` | Transform attributes (apply modes, failure handling) |
| `IR/Utils.cpp` | Transform utility functions (op matching, tracking) |
| `Transforms/InterpreterPass.cpp` | Pass that interprets a transform script against payload IR |
| `Transforms/TransformInterpreterUtils.cpp` | Utilities for interpreter execution |
| `Transforms/CheckUses.cpp` | Validates transform handle use-def correctness |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core transform ops, handle types, dialect |
| `Transforms/` | Interpreter pass, use checking, effect inference |
| `Interfaces/` | Transform op interface definitions |
| `DebugExtension/` | Debug ops for transform script tracing |
| `IRDLExtension/` | IRDL-based dynamic transform op extension |
| `LoopExtension/` | Loop-specific transform ops |
| `PDLExtension/` | PDL-based pattern rewriting via transform |
| `SMTExtension/` | SMT verification via transform |
| `TuneExtension/` | Tuning/autotuning support |
| `Utils/` | Shared transform utilities |

## For AI Agents

### Working In This Directory
- Transform ops operate on **handles** (`!transform.any_op`) that reference payload IR ops — not on the ops themselves.
- `transform.sequence` is the primary container; it sequences transform ops with failure-propagation semantics.
- Extension mechanism: other dialects (linalg, tensor, scf, etc.) register their transform ops via `TransformDialect::registerExtension<>()`.
- `InterpreterPass.cpp` drives execution: it finds the transform module, creates a `TransformState`, and applies ops top-down.
- `CheckUses.cpp` verifies that transform handles are used in dominance order and that consumed handles are not reused.
- Handle types are precise: `!transform.op<"linalg.matmul">` only matches matmul ops; `!transform.any_op` matches anything.
- Failed transforms can either silently fail (with `failure_propagation_mode = suppress`) or halt (default `propagate`).

### Common Patterns
- Implementing a new transform op: inherit from `TransformOpInterface`, implement `apply()` which receives `TransformRewriter` and `TransformState`.
- Pattern application: use `transform.apply_patterns` with a pattern set op to run rewrite patterns on payload IR.
- Matching: `transform.match.operation_name` / `transform.foreach` iterate over matched ops.

## Dependencies
- `mlir/IR`, `mlir/Dialect/Linalg`, `mlir/Dialect/SCF`, `mlir/Dialect/Vector`, `mlir/Dialect/Tensor` (via extensions)

<!-- MANUAL: -->
