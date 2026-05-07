<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend/HLSL

## Purpose

Provides HLSL (High-Level Shading Language) IR utilities for DirectX compilation. Handles HLSL-specific code generation including resource binding, root signatures, constant buffers, and shader-specific lowering to LLVM IR.

## Key Files

| File | Description |
|------|-------------|
| `CBuffer.cpp` | Constant buffer handling and layout |
| `HLSLBinding.cpp` | Resource binding management for textures, samplers, buffers |
| `HLSLResource.cpp` | HLSL resource representation and validation |
| `HLSLRootSignature.cpp` | Root signature construction for DirectX shaders |
| `RootSignatureMetadata.cpp` | Root signature metadata encoding |
| `RootSignatureValidations.cpp` | Root signature validation and error checking |

## For AI Agents

### Working In This Directory

1. Understand DirectX root signature concepts and resource binding model
2. Consult HLSL specification and DirectX documentation
3. Test shader compilation with various resource configurations
4. Verify root signature generation matches DirectX expectations
5. Validate constant buffer packing and alignment rules

### Key Patterns

- HLSL resources mapped to DirectX descriptor table slots
- Root signatures define resource binding layout for GPU
- Constant buffers require padding alignment to 16 bytes
- Resource validation performed during compilation

## Dependencies

### Internal
- Depends on: LLVM IR, Frontend utilities
- Used by: DirectX/clang-dxc shader compilation

<!-- MANUAL: -->
