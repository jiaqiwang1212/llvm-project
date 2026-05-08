<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToROCDLLibraryCalls Conversion

## Purpose
Lowers Complex dialect transcendental ops to ROCm device library (ROCDL) function calls. Converts complex math ops to `__ocml_*` device library functions available on AMD GPUs.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToROCDLLibraryCalls.cpp` | Patterns mapping complex ops to AMD OCML device library calls |

## For AI Agents

### Working In This Directory
- AMD GPUs use the OCML library (`__ocml_cexp_f32`, etc.) for complex math in device code — this pass emits calls to those functions.
- Function declarations reference OCML symbol names; the actual library is linked by the ROCm toolchain.
- Must be used in GPU device code compilation pipelines, not host code.

## Dependencies
- Source: `mlir/Dialect/Complex`
- Target: `mlir/Dialect/LLVMIR`, OCML library (AMD ROCm)

<!-- MANUAL: -->
