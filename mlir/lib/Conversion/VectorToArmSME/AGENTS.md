<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToArmSME

## Purpose
Implements the Vector to ArmSME lowering pass. Converts `vector` dialect operations on 2D scalable tiles into Arm SME (Scalable Matrix Extension) dialect ops for hardware matrix accumulator operations.

## Key Files
| File | Description |
|------|-------------|
| `VectorToArmSME.cpp` | Core conversion patterns from vector ops to ArmSME ops |
| `VectorToArmSMEPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateVectorToArmSMEPatterns()`
- SME tile types use scalable dimensions (`[?x?xf32]` with vscale); vector shapes must be tile-sized

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `vector.outerproduct` on scalable vectors → `arm_sme.outerproduct` accumulation ops

## Dependencies
- Headers: `include/mlir/Conversion/VectorToArmSME/`
- Source dialect: `lib/Dialect/Vector/`
- Target dialect: `lib/Dialect/ArmSME/`

<!-- MANUAL: -->
