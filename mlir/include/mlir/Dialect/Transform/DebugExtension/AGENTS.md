<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform DebugExtension

## Purpose
Debug and diagnostic transform ops: printing handles, emitting remarks, and inserting IR printing points into a Transform script for debugging purposes.

## Key Files
| File | Description |
|------|-------------|
| `DebugExtension.h` | Extension registration declaration |
| `DebugExtensionOps.h` | Op class declarations |
| `DebugExtensionOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- `transform.debug.emit_remark_at` prints a diagnostic at the targeted op's location
- `transform.debug.emit_param_as_remark` prints a transform parameter value
- Op names follow `transform.debug.*` convention

## Dependencies
- Depends on: Transform IR

<!-- MANUAL: -->
