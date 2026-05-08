<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PDLInterp IR

## Purpose
Core op definitions for the PDLInterp (PDL Interpreter) dialect: matching control-flow ops, type/attr/operand extraction ops, and rewrite application ops.

## Key Files
| File | Description |
|------|-------------|
| `PDLInterp.h` | Op class declarations |
| `PDLInterpOps.td` | ODS op definitions for all interpreter ops |

## For AI Agents

### Working In This Directory
- `pdl_interp.finalize` marks end of a successful match/rewrite sequence
- `pdl_interp.switch_type`, `pdl_interp.check_operand_count` etc. drive the match FSM

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
