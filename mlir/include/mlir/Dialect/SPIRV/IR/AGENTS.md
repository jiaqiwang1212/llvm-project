<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRV IR

## Purpose
Core op, type, attribute, and enum definitions for the SPIRV dialect. Organized by SPIR-V instruction category (arithmetic, control flow, memory, GL extensions, etc.).

## Key Files
| File | Description |
|------|-------------|
| `SPIRVDialect.h` | Dialect class declaration |
| `SPIRVBase.td` | Dialect definition, base classes, and availability system |
| `SPIRVOps.h` | Op class declarations |
| `SPIRVOps.td` | Top-level ODS include for all SPIRV ops |
| `SPIRVAttributes.h` | Attribute class declarations |
| `SPIRVAttributes.td` | ODS attribute definitions |
| `SPIRVTypes.h` | SPIR-V type class declarations |
| `SPIRVEnums.h` | Generated enum declarations |
| `SPIRVAvailability.td` | Op availability constraint definitions |
| `SPIRVArithmeticOps.td` | Arithmetic op definitions |
| `SPIRVControlFlowOps.td` | Control flow op definitions |
| `SPIRVMemoryOps.td` | Memory op definitions |
| `SPIRVGLOps.td` | GLSL450 extension op definitions |
| `SPIRVGroupOps.td` | Group/subgroup op definitions |
| `SPIRVCooperativeMatrixOps.td` | Cooperative matrix op definitions |
| `SPIRVOpTraits.h` | Op trait declarations |
| `SPIRVGLCanonicalization.h` | GLSL op canonicalization patterns |
| `TargetAndABI.h` | SPIR-V target environment and ABI helpers |
| `SPIRVTosaOps.h` | TOSA-level SPIRV op declarations |

## For AI Agents

### Working In This Directory
- `SPIRVAvailability.td` declares capability/extension requirements for each op
- `TargetAndABI.h` is the primary API for querying target capability support

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
