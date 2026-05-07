<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IRPrinter

## Purpose
Implements IR printing passes that output LLVM IR in human-readable format at various optimization stages. Essential for debugging and understanding compiler transformations.

## Key Files
| File | Description |
|------|-------------|
| `IRPrintingPasses.cpp` | IR printing pass implementations |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Print IR before/after optimization passes
- Function and module-level printing
- Integration with pass manager for pipeline debugging
- Output formatting for human readability

## Dependencies

### Internal
- `llvm/lib/IR/` — IR representation
- `llvm/lib/Pass/` — pass infrastructure

<!-- MANUAL: -->
