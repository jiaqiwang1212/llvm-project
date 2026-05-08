<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform TuneExtension

## Purpose
Transform dialect extension for tuning and auto-tuning: ops that parameterize transform sequences with tunable values, enabling integration with external auto-tuners and search frameworks.

## Key Files
| File | Description |
|------|-------------|
| `TuneExtension.h` | Extension registration declaration |
| `TuneExtensionOps.h` | Op class declarations |
| `TuneExtensionOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.tune.*` convention
- Tunable parameters are exposed as transform param types that can be varied externally

## Dependencies
- Depends on: Transform IR

<!-- MANUAL: -->
