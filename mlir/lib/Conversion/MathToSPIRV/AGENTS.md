<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToSPIRV Conversion

## Purpose
Lowers Math dialect ops to SPIR-V dialect ops. Converts transcendental and other math ops to SPIR-V GLSL extended instruction set ops or OpenCL extended instruction set ops.

## Key Files
| File | Description |
|------|-------------|
| `MathToSPIRV.cpp` | Patterns mapping math ops to spirv.GL.* or spirv.CL.* extended instruction ops |
| `MathToSPIRVPass.cpp` | Pass definition |

## For AI Agents

### Working In This Directory
- SPIR-V uses extended instruction sets: `spirv.GL.Exp` (GLSL 450) or `spirv.CL.exp` (OpenCL) for `math.exp`.
- Which extended instruction set is used depends on the SPIR-V execution environment (Vulkan → GL, OpenCL → CL).
- `SPIRVTargetEnvAttr` on the module controls which extended instructions are available.
- `math.sqrt` → `spirv.GL.Sqrt` or `spirv.CL.sqrt`; `math.fma` → `spirv.GL.Fma`.

## Dependencies
- Source: `mlir/Dialect/Math`
- Target: `mlir/Dialect/SPIRV`

<!-- MANUAL: -->
