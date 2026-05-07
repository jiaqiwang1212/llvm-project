<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# X86 Backend

## Purpose

The X86 backend implements code generation for x86 and x86-64 processors. It supports 16-bit (i8086), 32-bit (80386+), and 64-bit (x86-64) instruction sets, with extensive support for legacy compatibility and modern ISA extensions (SSE, AVX, AVX-512, BMI, etc.). This is one of the largest and most complex LLVM backends.

## Key Files

| File | Description |
|------|-------------|
| `X86TargetMachine.cpp` | TargetMachine implementation, subtarget creation |
| `X86TargetMachine.h` | TargetMachine class definition |
| `X86ISelDAGToDAG.cpp` | DAG-to-DAG instruction selection (very large, ~3000 lines) |
| `X86ISelDAGToDAG.h` | Instruction selection class definition |
| `X86ISelLowering.cpp` | Calling conventions, custom lowering, legalization (~4000 lines) |
| `X86ISelLowering.h` | ISelLowering interface |
| `X86AsmPrinter.cpp` | Assembly printing, MC emission |
| `X86AsmPrinter.h` | AsmPrinter implementation |
| `X86InstrInfo.cpp` | Instruction properties and patterns |
| `X86InstrInfo.h` | InstrInfo interface |
| `X86RegisterInfo.cpp` | Register file, frame index elimination |
| `X86RegisterInfo.h` | RegisterInfo interface |
| `X86FrameLowering.cpp` | Prologue/epilogue, stack frame layout |
| `X86FrameLowering.h` | FrameLowering interface |
| `X86.td` | Top-level TableGen definition |
| `X86InstrInfo.td` | Instruction definitions (~5000+ lines, very comprehensive) |
| `X86RegisterInfo.td` | Register definitions (GPR, XMM, YMM, ZMM, etc.) |
| `X86CallingConv.td` | Calling convention definitions (System V AMD64 ABI, Windows x64, etc.) |
| `X86Features.td` | Feature and predicate definitions (SSE, AVX, AVX-512, etc.) |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | X86 assembler parser for inline asm and .s files |
| `Disassembler/` | Binary-to-assembly disassembly for x86/x86-64 instructions |
| `GISel/` | GlobalISel infrastructure (machine-independent instruction selection) |
| `MCA/` | Machine Code Analyzer for performance modeling and throughput analysis |
| `MCTargetDesc/` | MC layer (Machine Code) target descriptions |
| `TargetInfo/` | Target info registration and initialization |

## For AI Agents

### Working In This Directory

- **Mode Variants**:
  - **32-bit mode**: Legacy x86 with 8 GP registers (EAX-EDI), addressing limited to 4GB
  - **64-bit mode (x86-64)**: Extended to 16 GP registers (RAX-R15), 64-bit addressing
  - **x32 ABI**: 64-bit ISA with 32-bit pointers and calling convention
  - Mode selection via subtarget features (Is32Bit, Is64Bit, IsX32)

- **Vector Extensions** (extensive, split across many .td files):
  - **MMX**: 64-bit vectors (mostly legacy)
  - **SSE/SSE2/SSE3**: 128-bit vectors (XMM registers)
  - **AVX/AVX2**: 256-bit vectors (YMM registers)
  - **AVX-512**: 512-bit vectors (ZMM registers) with mask registers (K0-K7)
  - Predicates in `X86Features.td` control availability of each extension

- **Instruction Selection Complexity**:
  - **DAG-to-DAG (ISelDAGToDAG.cpp)**: Pattern matching on DAG nodes; very large due to x86's complex ISA
  - **GlobalISel (GISel/)**: Machine-independent selection for better maintainability
  - Many custom lowering hooks in `X86ISelLowering.cpp` for x86-specific semantics

- **Addressing Modes**: x86 has rich addressing modes (base + index*scale + disp) handled by:
  - `X86InstrInfo.td` patterns using ComplexPattern predicates
  - Frame index elimination in `X86RegisterInfo.cpp`

- **TableGen Changes**: After modifying `.td` files, the build system automatically regenerates `.inc` files via `llvm-tblgen`. No manual regeneration needed.

- **Calling Conventions**: Multiple ABIs supported:
  - System V AMD64 ABI (Unix/Linux 64-bit)
  - Windows x64 ABI (Windows 64-bit)
  - 32-bit cdecl, stdcall, fastcall (Windows/legacy)
  - Defined in `X86CallingConv.td`, implemented in `X86ISelLowering.cpp` and `X86RegisterInfo.cpp`

### Common Patterns

- **Complex addressing modes**: `[base + index*scale + displacement]` with optional segment override; see `X86InstrInfo.td`.
- **Implicit operands**: Many x86 instructions have implicit register operands (e.g., DIV uses RAX/RDX); handled via tablegen implicit operands.
- **Partial register updates**: Writing to EAX implicitly zeroes the upper 32 bits of RAX; important for code correctness.
- **Packed integer/FP operations**: SSE/AVX SIMD instructions; definitions in `X86InstrInfo.td` with vector register classes.
- **Mask operations (AVX-512)**: K0-K7 mask registers control execution of masked instructions; special handling in `X86ISelDAGToDAG.cpp`.
- **Memory operands with immediates**: Many x86 instructions combine memory addressing with immediates (e.g., `add dword ptr [rax], 5`); patterns in `X86InstrInfo.td`.

## Dependencies

### Internal

- `lib/CodeGen/` — Target-independent instruction selection, register allocation, instruction scheduling
- `lib/CodeGen/GlobalISel/` — Machine-independent instruction selection framework (used by GISel/)
- `lib/MC/` — Machine Code layer for binary emission and assembler
- `lib/TargetParser/` — CPU feature parsing and compatibility checking
- `include/llvm/Target/` — Abstract TargetMachine, TargetInfo, TargetLowering interfaces

### External

- `llvm-tblgen` — TableGen compiler (invoked by build system to generate `.inc` files from `.td` definitions)

<!-- MANUAL: X86 backend is the largest and most mature; complexity arises from ISA evolution and multiple extensions. Changes to instruction selection or register allocation should be tested extensively on diverse code patterns. Refer to existing patterns before adding new ones. -->
