<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToSPIRV Conversion

## Purpose
Lowers the Arith dialect to SPIR-V dialect ops. Converts integer and floating-point arithmetic, comparisons, and casts to their SPIR-V equivalents, handling SPIR-V's signedness-explicit type system.

## Key Files
| File | Description |
|------|-------------|
| `ArithToSPIRV.cpp` | All conversion patterns from arith to spirv ops |

## For AI Agents

### Working In This Directory
- SPIR-V distinguishes signed vs. unsigned integer types (`spirv.IAdd` vs. `spirv.SDiv`/`spirv.UDiv`); conversions must map MLIR's signless integers to the appropriate SPIR-V ops.
- `arith.cmpi` with `slt`/`ult` predicate maps to `spirv.SLessThan`/`spirv.ULessThan` respectively.
- Float operations map directly: `arith.addf` → `spirv.FAdd`, etc.
- Uses `SPIRVTypeConverter` for type mapping.
- Capability requirements (e.g., `Float16` capability for f16 ops) are checked and added to the module.

## Dependencies
- Source: `mlir/Dialect/Arith`
- Target: `mlir/Dialect/SPIRV`
- Utility: `mlir/Conversion/SPIRVCommon`

<!-- MANUAL: -->
