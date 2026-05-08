<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenACC Transforms

## Purpose
Transformation passes for the OpenACC dialect: clause specialization, outlining, and other OpenACC-level restructuring passes.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `ACCSpecializePatterns.h` | Specialization rewrite patterns for ACC ops |

## For AI Agents

### Working In This Directory
- Implementations live in `lib/Dialect/OpenACC/Transforms/`

## Dependencies
- Depends on: OpenACC IR, SCF dialect

<!-- MANUAL: -->
