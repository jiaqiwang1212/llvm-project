<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Reducer/

## Purpose
Headers for the `mlir-reduce` test case reduction tool. Provides the reduction framework that shrinks failing MLIR programs to minimal reproducers by iteratively applying reduction patterns and testing with an external oracle. Modeled after `creduce`/`llvm-reduce` workflows.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Registration and factory functions for reducer passes |
| `Passes.td` | TableGen definitions for reducer passes |
| `ReductionNode.h` | `ReductionNode` — tree node representing a candidate reduced program |
| `ReductionPatternInterface.h` | C++ header for `ReductionPatternInterface` (generated from `.td`) |
| `DialectReductionPatternInterface.td` | TableGen definition of dialect-level reduction pattern hooks |
| `Tester.h` | `Tester` — wraps the external test script and caches interestingness results |
| `CMakeLists.txt` | Build rules |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Reduction passes implement `ReductionPatternInterface` to describe what IR elements they can remove.
- The `Tester` invokes an external script (e.g., a crash reproducer script) to check interestingness.
- Dialects can register custom reduction patterns via `DialectReductionPatternInterface`.
- The tool main entry point is in `mlir/Tools/mlir-reduce/`.

### Common Patterns
- Reduction pass: subclass a standard reduction pass (e.g., erase ops, simplify regions).
- Tester usage: `Tester tester(testScript, args); tester.isInteresting(module)`.

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext, Operation, Module)
- `mlir/Pass/` (Pass base class)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/` (raw_ostream, ToolOutputFile)

<!-- MANUAL: -->
