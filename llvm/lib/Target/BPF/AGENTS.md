<!-- Generated: 2026-04-23 | Updated: 2026-04-23 -->

# BPF Target Backend

## Purpose
LLVM backend for BPF (Berkeley Packet Filter / eBPF) — a sandboxed in-kernel VM used for networking, tracing, and security programs. Implements both the legacy DAG instruction-selection pipeline and the newer GlobalISel pipeline, plus an extensive set of BPF-specific IR and MI passes supporting CO-RE (Compile-Once Run-Everywhere), BTF debug info, and BPF verifier compatibility.

## Key Files

| File | Description |
|------|-------------|
| `BPF.h` | Top-level entry point; declares pass factory functions and pass initializers |
| `BPF.td` | Root TableGen file; includes all `.td` fragments |
| `BPFPassRegistry.def` | New PM pass registry for BPF function passes (`bpf-aspace-simplify`, `bpf-ir-peephole`, `bpf-preserve-static-offset`) |
| `BPFTargetMachine.h/.cpp` | Owns `BPFSubtarget` and `TLOF`; registers three triples (`bpf`, `bpfle`, `bpfbe`); wires both DAG ISel and GlobalISel pipelines; registers new PM callbacks via `registerPassBuilderCallbacks` |
| `BPFSubtarget.h/.cpp` | Feature flags for CPU versions: `HasJmpExt`, `HasJmp32`, `HasAlu32`, `HasLdsx/Movsx/Bswap/SdivSmod/Gotol/StoreImm/LoadAcqStoreRel/Gotox` (v4 ISA); owns GISel objects via `unique_ptr` |
| `BPFISelDAGToDAG.cpp` | DAG-to-DAG instruction selector |
| `BPFISelLowering.h/.cpp` | DAG lowering; handles ALU32/JMP32/JMP extensions; inline asm constraints; misaligned memory access policy; `BPF::R0` as exception pointer |
| `BPFInstrInfo.h/.cpp` | Instruction info |
| `BPFInstrInfo.td` | Instruction definitions and ISel patterns |
| `BPFInstrFormats.td` | Encoding format classes |
| `BPFCallingConv.td` | Calling convention |
| `BPFRegisterInfo.h/.cpp` | Register info |
| `BPFRegisterInfo.td` | Register and register-class definitions |
| `BPFFrameLowering.h/.cpp` | Stack frame management |
| `BPFSelectionDAGInfo.h/.cpp` | SelectionDAG target info hook |
| `BPFTargetTransformInfo.h` | Cost model for BPF (TTI) |
| `BPFTargetLoweringObjectFile.h/.cpp` | ELF object-file lowering |
| `BPFAsmPrinter.h/.cpp` | MC-level assembly printer; emits BTF sections |
| `BPFMCInstLower.h/.cpp` | Lowers `MachineInstr` to `MCInst` |
| `BPFCORE.h` | CO-RE shared constants: `BTFTypeIdFlag`, `PreserveTypeInfo`, `PreserveEnumValue`, `PreserveMemberOffset`, `PreserveFieldInfo` enums used across CO-RE passes |
| `BTFDebug.h/.cpp` | BTF (BPF Type Format) debug-info emitter; extends `DebugHandlerBase`; generates `.BTF` and `.BTF.ext` ELF sections |
| `BPFAbstractMemberAccess.cpp` | CO-RE pass: abstracts struct/union member accesses so the BPF loader can relocate field offsets at load time |
| `BPFPreserveDIType.cpp` | CO-RE pass: preserves `__builtin_btf_type_id()` debug-info type metadata |
| `BPFPreserveStaticOffset.cpp` | CO-RE pass: replaces `llvm.preserve.static.offset` + GEP + load/store with `llvm.bpf.getelementptr.and.load/store` for verifier-static-offset requirement |
| `BPFASpaceCastSimplifyPass.cpp` | IR pass: folds redundant `addrspacecast → GEP → addrspacecast` chains |
| `BPFIRPeephole.cpp` | IR-level peephole optimizations |
| `BPFCheckAndAdjustIR.cpp` | Pre-codegen IR checker/adjuster: validates no relocation globals in PHI nodes; removes `__builtin_bpf_passthrough` and `llvm.bpf.getelementptr.and.{load,store}` builtins; casts non-zero address-space pointers to AS0 |
| `BPFAdjustOpt.cpp` | Post-ISel optimization adjustments |
| `BPFMIChecking.cpp` | Machine IR safety checker (`BPFMIPreEmitChecking`) |
| `BPFMIPeephole.cpp` | Machine IR peephole pass (`BPFMIPeephole`, `BPFMIPreEmitPeephole`) |
| `BPFMISimplifyPatchable.cpp` | Machine IR pass to simplify patchable instructions (`BPFMISimplifyPatchable`) |
| `CMakeLists.txt` | Build rules; links `BPFAsmPrinter`, `BPFCodeGen`, `BPFDesc`, `BPFInfo` |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Parses BPF assembly text into MCInst (see `AsmParser/AGENTS.md`) |
| `Disassembler/` | Decodes BPF binary into MCInst (see `Disassembler/AGENTS.md`) |
| `GISel/` | GlobalISel pipeline: call lowering, legalizer, instruction selector, register bank (see `GISel/AGENTS.md`) |
| `MCTargetDesc/` | MC-layer: code emitter, asm backend, ELF writer, inst printer, fixups, assembler info (see `MCTargetDesc/AGENTS.md`) |
| `TargetInfo/` | LLVM target registry — exposes `bpf`, `bpfle`, `bpfbe` target singletons (see `TargetInfo/AGENTS.md`) |

## For AI Agents

### Working In This Directory

- **CO-RE passes** (`BPFAbstractMemberAccess`, `BPFPreserveDIType`, `BPFPreserveStaticOffset`) run before standard optimizations; they insert special intrinsics that later passes (`BPFCheckAndAdjustIR`) clean up. Do not remove CO-RE intrinsics outside these passes.
- **BTF debug info** is emitted by `BTFDebug` (called from `BPFAsmPrinter`). If adding new types or global variables, check whether BTF type entries need to be added in `BTFDebug.cpp`.
- **Three triples**: `bpf` (host-endian), `bpfle` (little-endian), `bpfbe` (big-endian) all use the same `BPFTargetMachine`. Endianness is encoded in the data layout string.
- **Dual ISel**: both DAG ISel (`BPFISelDAGToDAG`) and GlobalISel (`GISel/`) are active. New instruction patterns typically go in `BPFInstrInfo.td` (DAG ISel); GISel legalizer rules go in `GISel/BPFLegalizerInfo.cpp`.
- **New PM passes** must be registered in both `BPFPassRegistry.def` and `BPFTargetMachine::registerPassBuilderCallbacks`.
- **BPF verifier constraints**: loads/stores to context structs (e.g. `__sk_buff`) must use static immediate offsets — this is why `BPFPreserveStaticOffset` exists. Do not introduce GEPs with non-constant offsets to context pointers.

### Testing Requirements

- `llc -march=bpf` to verify codegen; `llvm-objdump -d` to inspect BTF sections.
- CO-RE pass tests live in `llvm/test/CodeGen/BPF/` (e.g. `BTF/`, `CORE/` subdirectories).
- `llvm-readelf --notes` or `bpftool btf dump` to inspect emitted BTF.

### Common Patterns

- BPF-specific intrinsics are declared in `llvm/include/llvm/IR/IntrinsicsBPF.td`.
- `BPFCoreSharedInfo` (in `BPFCORE.h`) holds enums shared between the IR passes and the MC/BTF layer; add new CO-RE relocation kinds here.
- Pass names follow `BPF{Function,Machine}*` convention; Machine IR passes also get `initializeBPF*Pass(PR)` calls in `LLVMInitializeBPFTarget`.

## Dependencies

### Internal
- `MCTargetDesc/` — register/instruction enums, code emission
- `TargetInfo/` — target singletons
- `GISel/` — GlobalISel hooks owned by `BPFSubtarget`
- `llvm/DebugInfo/BTF/` — BTF binary format constants used by `BTFDebug`

### External
- `llvm/CodeGen/` — `SelectionDAGISel`, `TargetLowering`, `GlobalISel/*`
- `llvm/IR/IntrinsicsBPF.td` — BPF-specific intrinsic declarations
- `llvm/Passes/PassBuilder.h` — new PM registration

<!-- MANUAL: -->
