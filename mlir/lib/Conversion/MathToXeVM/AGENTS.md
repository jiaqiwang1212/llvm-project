<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToXeVM Conversion

## Purpose
Lowers Math dialect ops to XeVM dialect intrinsics for Intel GPU targets. Converts math transcendental ops to Intel Xe GPU math intrinsic calls.

## Key Files
| File | Description |
|------|-------------|
| `MathToXeVM.cpp` | Patterns mapping math ops to XeVM math intrinsics |

## For AI Agents

### Working In This Directory
- Maps math ops to Intel Xe GPU specific intrinsics in the XeVM dialect.
- Part of the Intel GPU lowering pipeline: Math → XeVM → LLVM SPIR-V backend.
- Used in conjunction with `XeGPUToXeVM` and `XeVMToLLVM` for full Intel GPU lowering.

## Dependencies
- Source: `mlir/Dialect/Math`
- Target: `mlir/Dialect/XeVM` (Intel GPU intrinsics)

<!-- MANUAL: -->
