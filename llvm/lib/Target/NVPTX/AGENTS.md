<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVPTX Backend

## Purpose
Targets NVIDIA PTX (Parallel Thread Execution), LLVM's IR for NVIDIA GPUs. Used for CUDA compilation and GPU compute workloads on GeForce, Tesla, and other NVIDIA architectures.

## Key Files
| File | Description |
|------|-------------|
| `NVPTXTargetMachine.cpp` | TargetMachine implementation |
| `NVPTXISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `NVPTXISelLowering.cpp` | Calling convention and custom DAG lowering |
| `NVPTXAsmPrinter.cpp` | PTX assembly output |
| `NVPTXInstrInfo.cpp` | Instruction info implementation |
| `NVPTXRegisterInfo.cpp` | Register info implementation |
| `NVPTXFrameLowering.cpp` | Frame lowering for GPU kernels |
| `NVPTX.td` | Main target definition |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `MCTargetDesc/` | MC target description |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- NVPTXISelLowering.cpp handles CUDA calling conventions and custom DAG lowering
- NVPTXAsmPrinter.cpp generates PTX assembly suitable for NVIDIA driver
- GPU-specific passes like NVPTXAllocaHoisting.cpp for memory optimization
- NVPTXTargetTransformInfo.cpp provides GPU cost modeling

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
