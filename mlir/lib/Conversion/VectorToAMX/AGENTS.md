<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToAMX

## Purpose
Implements the Vector to AMX lowering pass. Converts `vector` dialect tile matrix ops into Intel AMX (Advanced Matrix Extensions) intrinsic calls for hardware-accelerated matrix multiplication on x86.

## Key Files
| File | Description |
|------|-------------|
| `VectorToAMX.cpp` | Conversion patterns from vector tile ops to AMX intrinsics |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateVectorToAMXConversionPatterns()`
- AMX tiles are fixed-size 2D register tiles; vector shapes must match AMX constraints

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- AMX intrinsics (`amx.tile_load`, `amx.tdpbf16ps`, etc.) require tile configuration setup

## Dependencies
- Headers: `include/mlir/Conversion/VectorToAMX/`
- Source dialect: `lib/Dialect/Vector/`
- Target dialect: `lib/Dialect/X86Vector/` (AMX ops)

<!-- MANUAL: -->
