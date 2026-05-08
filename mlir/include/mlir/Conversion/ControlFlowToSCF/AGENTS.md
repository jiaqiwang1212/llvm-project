<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlowToSCF

## Purpose
Declares the ControlFlow to SCF lifting pass. Lifts unstructured cf dialect branch ops back into structured SCF (if/while) constructs where possible.

## Key Files
| File | Description |
|------|-------------|
| `ControlFlowToSCF.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ControlFlowToSCF.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ControlFlowToSCF/`

## Dependencies
- Source dialect: `include/mlir/Dialect/ControlFlow/`
- Target dialect: `include/mlir/Dialect/SCF/`

<!-- MANUAL: -->
