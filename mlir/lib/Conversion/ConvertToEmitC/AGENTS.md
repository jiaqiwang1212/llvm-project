<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ConvertToEmitC Conversion

## Purpose
Provides the umbrella pass that orchestrates conversion of multiple dialects to EmitC. Drives the combined EmitC lowering pipeline by applying all registered EmitC conversion patterns in a single dialect conversion.

## Key Files
| File | Description |
|------|-------------|
| `ConvertToEmitCPass.cpp` | Top-level pass that applies all EmitC conversion patterns |

## For AI Agents

### Working In This Directory
- This is a pipeline entry point, not a conversion itself — it collects patterns from `ArithToEmitC`, `FuncToEmitC`, `SCFToEmitC`, `MathToEmitC`, `MemRefToEmitC`, and applies them together.
- Uses `populateXxxToEmitCConversionPatterns()` functions from each contributing conversion.
- Prefer this pass over running individual EmitC conversions separately to ensure consistent type conversion.

## Dependencies
- Source: Multiple dialects (Arith, Func, SCF, Math, MemRef)
- Target: `mlir/Dialect/EmitC`

<!-- MANUAL: -->
