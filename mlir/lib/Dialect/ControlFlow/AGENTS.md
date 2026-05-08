<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlow Dialect

## Purpose
Implements the ControlFlow dialect — the low-level structured and unstructured control flow primitives: `cf.br`, `cf.cond_br`, `cf.switch`, `cf.assert`. These are the target for SCF lowering and the source for LLVM IR branch lowering.

## Key Files
| File | Description |
|------|-------------|
| `IR/ControlFlowOps.cpp` | Op implementations: branch, conditional branch, switch, assert |
| `Transforms/StructuralTypeConversions.cpp` | Type conversion patterns for branch block arguments |
| `Transforms/BufferizableOpInterfaceImpl.cpp` | Bufferization interface for cf ops |
| `Transforms/BufferDeallocationOpInterfaceImpl.cpp` | Buffer deallocation interface |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core branch and control-flow ops |
| `Transforms/` | Type conversions and bufferization support |

## For AI Agents

### Working In This Directory
- `cf.cond_br` has two successor blocks with separate block argument lists — verifier checks that true/false operand counts match successor block argument counts.
- `StructuralTypeConversions.cpp` is critical for dialect conversion: it handles updating branch operand types when converting from a higher-level type (e.g., tensor) to a lower-level type (e.g., memref).
- `cf.assert` can be lowered to a conditional abort or removed depending on the pass configuration.

### Common Patterns
- Block argument type conversion: pattern rewrites `cf.br` to update successor operand types during `applyDialectConversion`.
- Verifiers use `Block::getArguments()` to validate branch target arity.

## Dependencies
- `mlir/IR`, `mlir/Dialect/Arith` (for assert condition), `mlir/Transforms/DialectConversion`

<!-- MANUAL: -->
