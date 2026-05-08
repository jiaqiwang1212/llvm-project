<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Linalg Utils

## Purpose
Utility functions for Linalg dialect: loop generation helpers, fusion utilities, indexing map queries, and operand/result shape helpers.

## Key Files
| File | Description |
|------|-------------|
| `Utils.h` | Linalg utility functions (loop gen, fusion, operand helpers) |

## For AI Agents

### Working In This Directory
- `generateLoopNest` and related helpers are used when lowering linalg ops to loop nests
- Shape inference helpers query indexing maps for static/dynamic dimension information

## Dependencies
- Depends on: Linalg IR, Affine dialect, SCF dialect

<!-- MANUAL: -->
