<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIR-V Backend

## Purpose
Targets SPIR-V (Standard Portable Intermediate Representation - Vulkan), the GPU IR for Vulkan and OpenCL compute. Used for shader and compute kernel compilation to portable GPU code.

## Key Files
| File | Description |
|------|-------------|
| `SPIRVTargetMachine.cpp` | TargetMachine implementation |
| `SPIRVISelLowering.cpp` | Calling convention and custom DAG lowering |
| `SPIRVAsmPrinter.cpp` | SPIR-V binary output |
| `SPIRVInstrInfo.cpp` | Instruction info implementation |
| `SPIRVRegisterInfo.cpp` | Register info implementation |
| `SPIRV.td` | Main target definition |
| `SPIRVBuiltins.td` | Built-in function definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Analysis/` | SPIR-V-specific analysis passes |
| `MCTargetDesc/` | MC target description |

## For AI Agents

### Working In This Directory
- Edit SPIRV.td and SPIRVBuiltins.td for instruction/builtin changes
- SPIRVISelLowering.cpp handles calling conventions and custom DAG lowering
- SPIRVAsmPrinter.cpp generates SPIR-V binary format
- SPIRVCallLowering.cpp manages function calls
- GPU compute and shader specific optimizations in Analysis/

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
