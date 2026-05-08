<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenACCToSCF

## Purpose
Implements the OpenACC to SCF lowering pass. Converts OpenACC loop and data constructs into SCF (Structured Control Flow) dialect ops as an intermediate step toward full code generation.

## Key Files
| File | Description |
|------|-------------|
| `OpenACCToSCF.cpp` | Conversion patterns from OpenACC ops to SCF ops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateOpenACCToSCFConversionPatterns()`
- SCF `for`/`if` ops replace OpenACC structured loop and conditional constructs

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- This pass is typically run before SCFToControlFlow or SCFToOpenMP

## Dependencies
- Headers: `include/mlir/Conversion/OpenACCToSCF/`
- Source dialect: `lib/Dialect/OpenACC/`
- Target dialect: `lib/Dialect/SCF/`

<!-- MANUAL: -->
