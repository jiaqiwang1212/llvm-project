<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IRDL Dialect

## Purpose
IR Definition Language dialect. Provides ops for defining new MLIR dialects, operations, types, and attributes dynamically within MLIR IR itself. Enables dialect definition without writing C++ code.

## Key Files
| File | Description |
|------|-------------|
| `IRDLLoading.h` | Functions to load IRDL dialect definitions into a context |
| `IRDLSymbols.h` | Symbol resolution for IRDL-defined entities |
| `IRDLVerifiers.h` | Verification utilities for IRDL-defined constraints |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, interface, and trait definitions (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- IRDL dialects are loaded at runtime via `IRDLLoading.h`; they do not require recompilation
- `IRDLVerifiers.h` provides constraint checking for dynamically-defined types/attrs

### Common Patterns
- Op names: `irdl.dialect`, `irdl.operation`, `irdl.type`, `irdl.attribute`, `irdl.constraint`
- Used by the Transform dialect's IRDL extension for pattern matching

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
