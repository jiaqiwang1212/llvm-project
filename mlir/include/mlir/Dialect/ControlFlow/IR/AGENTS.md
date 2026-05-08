<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlow IR

## Purpose
Core op definitions for the ControlFlow dialect: branches, assert, and switch ops.

## Key Files
| File | Description |
|------|-------------|
| `ControlFlow.h` | Dialect class declaration |
| `ControlFlowOps.h` | Op class declarations |
| `ControlFlowOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- Edit `ControlFlowOps.td` to add new control-flow ops
- All ops are block terminators or assertions; they do not return values to the next op

## Dependencies
- No dialect dependencies

<!-- MANUAL: -->
