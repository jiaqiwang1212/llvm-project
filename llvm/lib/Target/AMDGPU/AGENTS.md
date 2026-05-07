<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AMDGPU Backend

## Purpose
Compiles LLVM IR to AMD GPU machine code (GCN and RDNA architectures). Supports HIP, OpenCL, and general compute/graphics workloads on AMD Radeon and EPYC accelerator GPUs.

## Key Files
| File | Description |
|------|-------------|
| `AMDGPU.td` | Architecture features, register/instruction definitions |
| `AMDGPUISelLowering.cpp` | DAG lowering, calling conventions, custom operations |
| `AMDGPUISelDAGToDAG.cpp` | Instruction selection patterns |
| `AMDGPUAsmPrinter.cpp` | Assembly output and code generation |
| `AMDGPUCallLowering.cpp` | Call argument/return lowering (GlobalISel) |
| `AMDGPUTargetMachine.cpp` | Backend entry point and pipeline setup |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Parses AMD GPU assembly (.s) into MCInst |
| `Disassembler/` | Decodes binary machine code to assembly |
| `MCTargetDesc/` | MC layer: instruction encoding, register info |
| `TargetInfo/` | Target registration and metadata |
| `Utils/` | Shared utilities (bit tracking, peephole opts) |
| `MCA/` | Machine Code Analyzer for performance modeling |

## For AI Agents

### Working In This Directory
- Edit `.td` files for ISA changes, rebuild with `llvm-tblgen`
- Modify `AMDGPUISelLowering.cpp` for custom lowering rules
- Use `AMDGPUCallLowering.cpp` for GlobalISel integration
- Compiler intrinsics in MCTargetDesc/ for encoding

### Key Compilation Steps
1. TableGen builds register/instruction defs from `.td` files
2. DAG lowering converts high-level operations to low-level forms
3. Instruction selection matches patterns and emits machine code
4. Register allocation and scheduling follow standard LLVM pipeline
5. ASM printer outputs GCN/RDNA assembly or binary

## Dependencies

### Internal
- `lib/CodeGen/` — Core LLVM code generation
- `lib/MC/` — Machine code layer (encoding, assembly)
- `lib/IR/` — LLVM IR representation

### External
- AMD ISA manuals (GCN, RDNA)
- HSAIL/HIP specifications

