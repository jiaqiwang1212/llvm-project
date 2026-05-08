<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform SMTExtension

## Purpose
Transform dialect extension that integrates SMT verification with Transform scripts. Enables verification-driven transformations and formal property checking as transform ops.

## Key Files
| File | Description |
|------|-------------|
| `SMTExtension.h` | Extension registration declaration |
| `SMTExtensionOps.h` | Op class declarations |
| `SMTExtensionOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.smt.*` convention
- Allows SMT formulas to guard or drive transform decisions

## Dependencies
- Depends on: Transform IR, SMT dialect

<!-- MANUAL: -->
