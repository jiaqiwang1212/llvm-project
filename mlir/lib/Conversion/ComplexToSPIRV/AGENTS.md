<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToSPIRV Conversion

## Purpose
Lowers Complex dialect ops to SPIR-V dialect ops. Converts complex arithmetic to SPIR-V composite extract/insert and arithmetic ops, since SPIR-V lacks native complex types.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToSPIRV.cpp` | Conversion patterns from complex ops to spirv |
| `ComplexToSPIRVPass.cpp` | Pass definition and registration |

## For AI Agents

### Working In This Directory
- SPIR-V has no native complex type; `complex<f32>` is represented as `!spirv.array<2 x f32>` or a struct.
- Component extraction uses `spirv.CompositeExtract`; reconstruction uses `spirv.CompositeConstruct`.
- Arithmetic is decomposed: `complex.add` → two `spirv.FAdd` + composite ops.

## Dependencies
- Source: `mlir/Dialect/Complex`
- Target: `mlir/Dialect/SPIRV`

<!-- MANUAL: -->
