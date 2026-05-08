<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCF IR

## Purpose
Core op and interface definitions for the SCF dialect: `scf.for`, `scf.while`, `scf.if`, `scf.forall`, `scf.parallel`, `scf.reduce`, `scf.yield`, and device-mapping interfaces.

## Key Files
| File | Description |
|------|-------------|
| `SCF.h` | Op class declarations |
| `SCFOps.td` | ODS op definitions |
| `DeviceMappingInterface.h` | Device mapping interface declarations (GPU thread/block mapping) |
| `DeviceMappingInterface.td` | ODS device mapping interface definitions |
| `ValueBoundsOpInterfaceImpl.h` | ValueBounds impls for scf.for/forall ops |

## For AI Agents

### Working In This Directory
- `scf.forall` supports device mapping via `DeviceMappingInterface` attributes
- Edit `SCFOps.td` to add new structured control flow ops

## Dependencies
- Depends on: Arith/Index types (for loop bounds and step)

<!-- MANUAL: -->
