<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PDLToPDLInterp

## Purpose
Implements the PDL to PDLInterp lowering pass. Compiles high-level PDL (Pattern Description Language) rewrite rules into efficient PDLInterp bytecode instructions executed by the pattern interpreter at runtime.

## Key Files
| File | Description |
|------|-------------|
| `PDLToPDLInterp.cpp` | Main lowering pass orchestrating predicate compilation and code generation |
| `Predicate.cpp` / `Predicate.h` | Predicate representation for matching conditions |
| `PredicateTree.cpp` / `PredicateTree.h` | Tree structure for organizing and merging match predicates |
| `RootOrdering.cpp` / `RootOrdering.h` | Ordering heuristics for root operation selection in match trees |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- This conversion is more complex than typical dialect lowerings — it compiles a pattern DSL
- `PredicateTree` merges shared prefix predicates across multiple PDL patterns for efficiency
- `RootOrdering` selects which op in a PDL pattern to use as the match root

### Common Patterns
- PDL `pdl.pattern` ops are analyzed to extract match predicates and rewrite actions
- The resulting `pdl_interp` bytecode uses a branching interpreter model
- Predicate deduplication is key to generating compact interpreter programs

## Dependencies
- Headers: `include/mlir/Conversion/PDLToPDLInterp/`
- Source dialect: `lib/Dialect/PDL/`
- Target dialect: `lib/Dialect/PDLInterp/`

<!-- MANUAL: -->
