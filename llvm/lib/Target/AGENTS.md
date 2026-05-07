<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Target Backends (llvm/lib/Target)

## Purpose
Contains architecture-specific backend implementations for LLVM code generation. Each subdirectory implements a complete compiler backend for a target architecture, including instruction selection, register allocation, assembly printing, and object-file emission.

## Target Backends

| Target | Directory | Purpose |
|--------|-----------|---------|
| **AArch64** | `AArch64/` | ARMv8 64-bit architecture backend |
| **AMDGPU** | `AMDGPU/` | AMD GPU (RDNA, CDNA) backend |
| **ARC** | `ARC/` | ARC (Synopsys ARC processor) backend |
| **ARM** | `ARM/` | ARM 32-bit architecture backend |
| **AVR** | `AVR/` | Atmel AVR 8-bit microcontroller backend |
| **BPF** | `BPF/` | Berkeley Packet Filter (eBPF) backend |
| **Cpu0** | `Cpu0/` | **Tutorial backend**: 32-bit RISC ISA modeled after MIPS; used in "Creating an LLVM Backend for the Cpu0 Architecture" (see `Cpu0/AGENTS.md`) |
| **CSKY** | `CSKY/` | C-SKY embedded processor backend |
| **DirectX** | `DirectX/` | DirectX shader compilation backend |
| **Hexagon** | `Hexagon/` | Qualcomm Hexagon VLIW processor backend |
| **Lanai** | `Lanai/` | Lanai processor backend |
| **LoongArch** | `LoongArch/` | Loongson LoongArch backend |
| **M68k** | `M68k/` | Motorola 68000 backend |
| **Mips** | `Mips/` | MIPS architecture backend (32-bit and 64-bit) |
| **MSP430** | `MSP430/` | Texas Instruments MSP430 microcontroller backend |
| **NVPTX** | `NVPTX/` | NVIDIA PTX (GPU) backend |
| **PowerPC** | `PowerPC/` | IBM PowerPC architecture backend |
| **RISCV** | `RISCV/` | RISC-V open-source ISA backend |
| **Sparc** | `Sparc/` | SPARC architecture backend |
| **SPIRV** | `SPIRV/` | SPIR-V shader format backend |
| **SystemZ** | `SystemZ/` | IBM System z (mainframe) backend |
| **VE** | `VE/` | NEC SX-Aurora Vector Engine backend |
| **WebAssembly** | `WebAssembly/` | WebAssembly (WASM) backend |
| **X86** | `X86/` | Intel/AMD x86 and x64 architecture backend |
| **XCore** | `XCore/` | XMOS XCore processor backend |
| **Xtensa** | `Xtensa/` | Tensilica Xtensa processor backend |

## Core Files

| File | Description |
|------|-------------|
| `CMakeLists.txt` | Build configuration; registers all target backends and common utilities |
| `Target.cpp` | Global target registry initialization |
| `TargetMachine.cpp` | Base `TargetMachine` implementation; instantiates architecture-specific passes |
| `TargetMachineC.cpp` | C API bindings for target machine creation and configuration |
| `TargetLoweringObjectFile.cpp` | Default ELF/Mach-O/COFF object file lowering; overridden by individual targets |
| `RegisterTargetPassConfigCallback.cpp` | Pass pipeline registration mechanism |
| `README.txt` | Target-independent optimization opportunities and known issues |

## For AI Agents

### Working In This Directory

- **Backend anatomy**: Each target subdirectory contains:
  - `TargetMachine.cpp/h` — entry point for code generation
  - `Subtarget.cpp/h` — architecture variants and feature flags
  - `InstrInfo.cpp/h` — instruction definitions and properties
  - `RegisterInfo.cpp/h` — register set and allocation constraints
  - `FrameLowering.cpp/h` — stack frame setup/teardown
  - `ISelLowering.cpp/h` — DAG lowering (high-level IR to machine DAG)
  - `ISelDAGToDAG.cpp/h` — DAG-to-DAG instruction selection
  - `*.td` files — TableGen instruction/register definitions
  - `AsmPrinter/` — assembly text output
  - `MCTargetDesc/` — MC-layer target description (registers, instructions for assembler/disassembler)
  - `TargetInfo/` — target registry

- **Adding a new target**: Copy an existing similar target (e.g., Cpu0 for RISC, ARM for 32-bit), modify the architecture-specific components, and register with `llvm::Target` via `TargetInfo/`.

- **TableGen pipeline**: `.td` files in each target are compiled at build time to generate `.inc` files included by `.cpp` files. Changes to `.td` require rebuild to take effect.

- **Pass insertion points**: Targets customize code generation via `createPassConfig()` in `TargetMachine`, inserting target-specific passes before/after the standard pipeline.

### Common Patterns

- **Subtarget feature flags**: Define ISA variants (e.g., "HasVFP" on ARM) in `Subtarget.td` and access via `getSubtargetImpl(Function)`.
- **Two-level hierarchy**: Base `TargetMachine` holds default implementation; derived classes (e.g., `Cpu0elTargetMachine`, `Cpu0ebTargetMachine`) configure endianness, ABI, etc.
- **Generic hooks**: `TargetLoweringObjectFile`, `TargetLowering`, `TargetInstrInfo` are abstract base classes with default implementations in `Target.cpp`; targets override as needed.

### Testing Requirements

- Each target should have a test suite under `llvm/test/CodeGen/{TargetName}/` with `.ll` or `.c` files and RUN lines.
- Build via `cmake --build build --target {TargetName}` to verify compilation.
- For a tutorial backend like Cpu0, the canonical test is `llc -march=cpu0 -filetype=obj test.bc -o test.o`.

## Dependencies

### Internal
- `llvm/CodeGen/` — selection DAG, register allocation, machine code generation
- `llvm/MC/` — machine code (assembly, object files)
- `llvm/IR/` — intermediate representation
- `llvm/Analysis/` — dataflow and dependency analysis
- `llvm/Support/` — utilities (StringRef, raw_ostream, etc.)

### Per-Target
Each target depends on:
- Its own `TargetInfo/`, `MCTargetDesc/`, `InstPrinter/` subdirectories
- LLVM code generation and analysis infrastructure

## Architecture Overview

Code generation pipeline for all targets:
1. **IR → DAG**: `SelectionDAGISel::runOnMachineFunction()` (ISelLowering + ISelDAGToDAG)
2. **DAG → MachineInstr**: Instruction selection and lowering
3. **MachineInstr → Machine**: Register allocation, scheduling, frame lowering
4. **Machine → Object**: Assembly printing and object-file emission via MC layer

Each target implements hooks into this pipeline via `TargetMachine`, `TargetLowering`, `TargetInstrInfo`, and `TargetRegisterInfo`.

<!-- MANUAL: -->
