<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToLLVM Conversion

## Purpose
Lowers Complex dialect ops to LLVM IR dialect. Converts complex arithmetic ops to sequences of LLVM float arithmetic on the real and imaginary components stored as LLVM struct types.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToLLVM.cpp` | Patterns lowering complex ops to llvm struct-based arithmetic |

## For AI Agents

### Working In This Directory
- `complex<f32>` maps to `!llvm.struct<(f32, f32)>` — real part first, imaginary second.
- Arithmetic ops decompose into component operations: `complex.add` → two `llvm.fadd` + struct reconstruction.
- Division uses `ComplexCommon/DivisionConverter` for numerically stable Smith's algorithm.
- `complex.re`/`complex.im` extraction maps to `llvm.extractvalue` with index 0/1.

## Dependencies
- Source: `mlir/Dialect/Complex`
- Target: `mlir/Dialect/LLVMIR`
- Utility: `mlir/Conversion/ComplexCommon`

<!-- MANUAL: -->
