<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRV Dialect

## Purpose
Implements the SPIR-V dialect — a faithful MLIR representation of the SPIR-V intermediate language targeting Vulkan, OpenCL, and OpenGL GPU shaders/kernels. Provides SPIR-V types, ops, serialization/deserialization, and SPIR-V-specific optimization passes.

## Key Files
| File | Description |
|------|-------------|
| `IR/SPIRVDialect.cpp` | Dialect registration, serialization/deserialization interface |
| `IR/SPIRVOps.cpp` | Core SPIR-V op implementations |
| `IR/SPIRVTypes.cpp` | SPIR-V type system: `!spirv.ptr`, `!spirv.struct`, `!spirv.array`, `!spirv.image`, etc. |
| `IR/SPIRVAttributes.cpp` | SPIR-V attribute definitions (decorations, capabilities) |
| `IR/SPIRVEnums.cpp` | SPIR-V enum definitions (storage class, execution model, etc.) |
| `IR/TargetAndABI.cpp` | Target environment and ABI utilities |
| `IR/SPIRVCanonicalization.cpp` | Canonicalization patterns |
| `Transforms/SPIRVConversion.cpp` | Type conversion utilities for SPIR-V lowering |
| `Transforms/LowerABIAttributesPass.cpp` | Lowers ABI attributes to explicit SPIR-V variable/decoration ops |
| `Transforms/DecorateCompositeTypeLayoutPass.cpp` | Adds layout decorations for struct types |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Full SPIR-V op set, type system, attributes, canonicalization |
| `Transforms/` | ABI lowering, layout decoration, WebGPU transforms, version updates |
| `Interfaces/` | SPIR-V op interface implementations |
| `Linking/` | SPIR-V module linking utilities |
| `Utils/` | Shared SPIR-V utilities |

## For AI Agents

### Working In This Directory
- SPIR-V has strict structural requirements: ops must be inside `spirv.module` which specifies `AddressingModel` and `MemoryModel`.
- `TargetAndABI.cpp` provides `SPIRVTargetEnvAttr` which encodes capability/extension requirements — always attach before lowering.
- Storage classes (Function, StorageBuffer, Workgroup, etc.) must match variable declarations; verifiers enforce this.
- `SPIRVConversion.cpp` provides `SPIRVTypeConverter` — use it in any pass lowering to SPIR-V.
- `DecorateCompositeTypeLayoutPass` must run before serialization: SPIR-V structs require explicit `Offset` decorations.
- Serialization/deserialization: `spirv::serialize()`/`spirv::deserialize()` convert between MLIR and SPIR-V binary.
- `Linking/` handles merging multiple SPIR-V modules (e.g., kernel + library).

### Common Patterns
- Op availability: `SPIRVOpAvailability.cpp` checks which capabilities/extensions each op requires.
- Type conversion: `SPIRVTypeConverter::convertType()` maps MLIR types to SPIR-V types using registered hooks.
- Struct layout: `VulkanLayoutUtils::decorateType()` adds required `Offset`/`ArrayStride`/`MatrixStride` decorations.

## Dependencies
- `mlir/Dialect/GPU`, `mlir/Dialect/Func`, `mlir/Dialect/Vector`, `mlir/Dialect/MemRef`

<!-- MANUAL: -->
