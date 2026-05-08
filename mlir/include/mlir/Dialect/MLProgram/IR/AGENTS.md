<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MLProgram IR

## Purpose
Core op, type, and attribute definitions for the MLProgram dialect.

## Key Files
| File | Description |
|------|-------------|
| `MLProgram.h` | Op class declarations |
| `MLProgramBase.td` | Dialect definition and base classes |
| `MLProgramOps.td` | ODS op definitions |
| `MLProgramAttributes.h` | Attribute class declarations |
| `MLProgramAttributes.td` | ODS attribute definitions |
| `MLProgramTypes.h` | Type class declarations |
| `MLProgramTypes.td` | ODS type definitions (token type) |

## For AI Agents

### Working In This Directory
- Edit `MLProgramOps.td` to add new ML program ops
- Token type is used for sequencing side-effectful global operations

## Dependencies
- Depends on: Func dialect, Tensor dialect types

<!-- MANUAL: -->
