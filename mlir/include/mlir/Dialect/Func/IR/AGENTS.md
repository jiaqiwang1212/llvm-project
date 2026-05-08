<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Func IR

## Purpose
Core op definitions for the Func dialect: `func.func`, `func.call`, `func.call_indirect`, `func.return`, `func.constant`.

## Key Files
| File | Description |
|------|-------------|
| `FuncOps.h` | Op class declarations |
| `FuncOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- `func.func` implements `FunctionOpInterface`, `CallableOpInterface`, `Symbol`
- Edit `FuncOps.td` to modify function or call op behavior

## Dependencies
- No dialect dependencies

<!-- MANUAL: -->
