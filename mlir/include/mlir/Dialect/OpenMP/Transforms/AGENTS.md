<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenMP Transforms

## Purpose
Transformation passes for the OpenMP dialect: legalization, privatization, and other OpenMP-specific restructuring before lowering to LLVM.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |

## For AI Agents

### Working In This Directory
- Implementations live in `lib/Dialect/OpenMP/Transforms/`

## Dependencies
- Depends on: OpenMP IR, LLVMIR dialect

<!-- MANUAL: -->
