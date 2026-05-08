<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tosa Dialect

## Purpose
Implements the TOSA (Tensor Operator Set Architecture) dialect — a standardized operator set for ML inference targeting edge/embedded devices. Defined by the TOSA spec, it provides arithmetic, activation, convolution, pooling, and data layout ops with strict type rules and quantization support.

## Key Files
| File | Description |
|------|-------------|
| `IR/TosaOps.cpp` | All TOSA op implementations and verifiers |
| `IR/TosaCanonicalizations.cpp` | Canonicalization patterns (constant folding, identity elimination) |
| `IR/TargetEnv.cpp` | TOSA profile/extension/level target environment |
| `IR/ShardingInterfaceImpl.cpp` | Sharding interface for distributed execution |
| `Transforms/TosaInferShapes.cpp` | Shape inference for TOSA ops |
| `Transforms/TosaValidation.cpp` | Validates IR against TOSA spec constraints |
| `Transforms/TosaDecomposeTransposeConv.cpp` | Decomposes `tosa.transpose_conv2d` to conv2d + arithmetic |
| `Transforms/TosaFolders.cpp` | Constant folding passes |
| `Transforms/TosaLayerwiseConstantFoldPass.cpp` | Layer-wise constant propagation |
| `Transforms/TosaProfileCompliance.cpp` | Checks ops against TOSA profile (Base/Main/MT) |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | TOSA ops, canonicalization, target environment, sharding |
| `Transforms/` | Shape inference, validation, decomposition, constant folding |
| `Utils/` | Shared TOSA utilities |

## For AI Agents

### Working In This Directory
- TOSA has strict type rules: integer types must be signless, and quantized ops carry explicit scale/zero-point attributes.
- `TosaValidation.cpp` enforces TOSA spec compliance — run it after lowering from higher-level frontends to verify conformance.
- `TosaInferShapes.cpp` is needed before lowering because TOSA ops have statically inferrable output shapes.
- Lowering pipeline: TOSA → `TosaToLinalg` / `TosaToArith` / `TosaToSCF` / `TosaToTensor` for compute, then standard lowering.
- `TargetEnv` encodes which TOSA profile/level is being targeted (Base Inference, Main Inference, Main Training).
- `TosaProfileCompliance.cpp` checks that only ops allowed in the target profile are present.

### Common Patterns
- Verifiers check that input/output types are rank-compatible and satisfy TOSA data type rules.
- Constant folding: `TosaFolders.cpp` evaluates ops with `ElementsAttr` operands at compile time.
- Decomposition: `TosaDecomposeDepthwise.cpp` rewrites `tosa.depthwise_conv2d` as `tosa.conv2d` with reshape.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/Dialect/Linalg`, `mlir/Dialect/Tensor`, `mlir/Dialect/SCF`

<!-- MANUAL: -->
