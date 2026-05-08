<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tosa IR

## Purpose
Core op, interface, shape-op, and compliance definitions for the TOSA dialect.

## Key Files
| File | Description |
|------|-------------|
| `TosaOps.h` | Op class declarations |
| `TosaOpBase.td` | Base classes for TOSA ops |
| `TosaOps.td` | ODS op definitions |
| `TosaShapeOps.td` | Shape-level op definitions |
| `TosaTypesBase.td` | Type base definitions |
| `TosaUtilOps.td` | Utility op definitions |
| `TosaInterfaces.td` | Interface definitions |
| `TosaDialectBytecode.td` | Bytecode serialization definitions |
| `TosaProfileCompliance.h` | TOSA profile compliance checking declarations |
| `TosaComplianceData.h.inc` | Generated compliance data |
| `TargetEnv.h` | TOSA target environment helpers |
| `ShardingInterfaceImpl.h` | Sharding interface implementations for TOSA ops |

## For AI Agents

### Working In This Directory
- TOSA compliance: ops have profile (Base Inference, Main Inference, etc.) requirements
- `TosaProfileCompliance.h` provides tools to check op legality for a given profile

## Dependencies
- Depends on: Tensor dialect types, Quant dialect types

<!-- MANUAL: -->
