<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# UBToSPIRV

## Purpose
Implements the UB (Undefined Behavior) to SPIR-V lowering pass. Converts `ub.poison` ops into SPIR-V `spirv.Undef` ops, representing undefined values in the SPIR-V execution model.

## Key Files
| File | Description |
|------|-------------|
| `UBToSPIRV.cpp` | Conversion patterns from ub.poison to spirv.Undef |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateUBToSPIRVConversionPatterns()`
- SPIRVTypeConverter handles element type lowering for undef-typed values

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- `ub.poison` → `spirv.Undef` with the SPIR-V-lowered result type
- SPIR-V undef semantics differ slightly from LLVM poison; values are unspecified but not propagating

## Dependencies
- Headers: `include/mlir/Conversion/UBToSPIRV/`
- Source dialect: `lib/Dialect/UB/`
- Target dialect: `lib/Dialect/SPIRV/`

<!-- MANUAL: -->
