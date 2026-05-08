<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# X86 Dialect

## Purpose
Implements the X86 dialect — exposes x86/x86-64 specific intrinsics and instructions not expressible in generic Vector or LLVM dialects: AVX/AVX2/AVX-512 vector operations, AMX (Advanced Matrix Extensions) tile ops, and FMA (fused multiply-add) patterns targeting Intel CPUs.

## Key Files
| File | Description |
|------|-------------|
| `IR/X86Dialect.cpp` | Dialect registration |
| `Transforms/AVXTranspose.cpp` | Lowers `vector.transpose` using AVX shuffle/blend intrinsics |
| `Transforms/LegalizeForLLVMExport.cpp` | Legalizes X86 ops for LLVM IR export |
| `Transforms/ShuffleVectorFMAOps.cpp` | Rewrites shuffle+mul patterns to FMA instructions |
| `Transforms/SinkVectorProducerOps.cpp` | Sinks vector producer ops to improve register allocation |
| `Transforms/VectorContractBF16ToFMA.cpp` | Lowers BF16 vector contracts to FMA sequences |
| `Transforms/VectorContractToAMXDotProduct.cpp` | Lowers vector contracts to AMX tile multiply instructions |
| `Transforms/VectorContractToFMA.cpp` | Lowers vector contracts to x86 FMA intrinsics |
| `Transforms/VectorContractToPackedTypeDotProduct.cpp` | Lowers contracts to VNNI packed dot-product instructions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | X86 dialect definition |
| `Transforms/` | AVX transpose lowering, FMA uplift, AMX lowering, vector legalization |
| `TransformOps/` | Transform dialect extension for X86 |
| `Utils/` | Shared X86 utilities |

## For AI Agents

### Working In This Directory
- X86 ops map to specific x86 intrinsics (`_mm256_*`, `_mm512_*`, `_tile_*`); they require target feature checks.
- `VectorContractToAMXDotProduct.cpp` targets AMX on Sapphire Rapids — requires tile configuration setup.
- `AVXTranspose.cpp` generates shuffle/blend sequences for transposing small vector blocks (4x4, 8x8, 16x16).
- `VectorContractToFMA.cpp` detects outer-product or dot-product patterns and emits x86 FMA3 intrinsics.
- `VNNI` (Vector Neural Network Instructions) in `VectorContractToPackedTypeDotProduct.cpp` require int8/bf16 packed operand layout.

### Common Patterns
- Contract lowering: match `vector.contract` by element type and tile size to select the right x86 instruction.
- FMA detection: look for `addf(mulf(a,b), c)` or `vector.contract` patterns and rewrite to `math.fma` / x86 FMA intrinsic.

## Dependencies
- `mlir/Dialect/Vector`, `mlir/Dialect/LLVMIR`, `mlir/Dialect/Arith`

<!-- MANUAL: -->
