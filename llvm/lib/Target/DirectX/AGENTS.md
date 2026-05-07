<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DirectX Backend

## Purpose
Targets DXIL (DirectX Intermediate Language) bytecode for HLSL/DirectX shader compilation. Used for GPU shader compilation in DirectX 12 and later.

## Key Files
| File | Description |
|------|-------------|
| `DirectXTargetMachine.cpp` | TargetMachine implementation |
| `DirectXTargetLowering.h` | Calling convention and DAG lowering |
| `DirectXAsmPrinter.cpp` | DXIL output |
| `DirectXInstrInfo.cpp` | Instruction info implementation |
| `DirectXRegisterInfo.cpp` | Register info implementation |
| `DirectXFrameLowering.h` | Frame lowering (minimal for shader targets) |
| `DirectX.td` | Main target definition |
| `DirectXSubtarget.cpp` | Subtarget feature handling |
| `CBufferDataLayout.cpp` | Constant buffer layout utilities |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `DirectXIRPasses/` | IR-level passes specific to DirectX |

## For AI Agents

### Working In This Directory
- Edit DirectXTargetLowering.h for calling convention or DAG lowering changes
- DirectXAsmPrinter.cpp generates DXIL bytecode
- DirectXIRPasses/ contains shader-specific transformations

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
