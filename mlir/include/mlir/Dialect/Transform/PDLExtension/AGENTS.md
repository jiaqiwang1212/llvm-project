<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform PDLExtension

## Purpose
Transform dialect extension that integrates PDL pattern matching with Transform scripts. Allows PDL-defined patterns to be applied as transform ops.

## Key Files
| File | Description |
|------|-------------|
| `PDLExtension.h` | Extension registration declaration |
| `PDLExtensionOps.h` | Op class declarations |
| `PDLExtensionOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.pdl.*` convention
- `transform.pdl.match_op_with_pdl_pattern` applies PDL pattern matching as a transform handle selector

## Dependencies
- Depends on: Transform IR, PDL dialect, PDLInterp dialect

<!-- MANUAL: -->
