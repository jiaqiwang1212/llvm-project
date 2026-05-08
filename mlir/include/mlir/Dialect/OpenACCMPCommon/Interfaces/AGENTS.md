<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenACCMPCommon Interfaces

## Purpose
Shared interface definitions for OpenACC and OpenMP dialects: atomic operation interfaces and common op interfaces used by both directive-based parallel programming models.

## Key Files
| File | Description |
|------|-------------|
| `AtomicInterfaces.h` | Atomic op interface declarations |
| `AtomicInterfaces.td` | ODS atomic interface definitions |
| `OpenACCMPOpsInterfaces.h` | Shared op interface declarations |
| `OpenACCMPOpsInterfaces.td` | ODS shared op interface definitions |

## For AI Agents

### Working In This Directory
- Implement these interfaces on both OpenACC and OpenMP atomic ops for unified handling

## Dependencies
- Used by: OpenACC IR, OpenMP IR

<!-- MANUAL: -->
