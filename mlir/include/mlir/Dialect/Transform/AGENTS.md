<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform Dialect

## Purpose
The Transform dialect provides a meta-programming framework for expressing and applying IR transformations as first-class MLIR IR. Transform scripts select IR handles and apply transformations, enabling reproducible, composable optimization sequences.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `DebugExtension/` | Debug and printing transform ops (see `DebugExtension/AGENTS.md`) |
| `Interfaces/` | Core transform interfaces (see `Interfaces/AGENTS.md`) |
| `IR/` | Core op, type, and dialect definitions (see `IR/AGENTS.md`) |
| `IRDLExtension/` | IRDL-based pattern matching extension (see `IRDLExtension/AGENTS.md`) |
| `LoopExtension/` | Loop-level transform ops (see `LoopExtension/AGENTS.md`) |
| `PDLExtension/` | PDL-based pattern matching extension (see `PDLExtension/AGENTS.md`) |
| `SMTExtension/` | SMT-based verification extension (see `SMTExtension/AGENTS.md`) |
| `Transforms/` | Transform interpreter passes (see `Transforms/AGENTS.md`) |
| `TuneExtension/` | Tuning/search transform ops (see `TuneExtension/AGENTS.md`) |
| `Utils/` | Transform utility functions (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Transform scripts are MLIR modules with a `transform.sequence` or `transform.named_sequence` root
- Handles (`!transform.any_op`, `!transform.op<"dialect.op">`) are the primary types
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- Every transform op implements `TransformOpInterface`
- Extension ops are registered per-dialect via `registerTransformExtension`

## Dependencies
- Core infrastructure; other dialects register extensions into it

<!-- MANUAL: -->
