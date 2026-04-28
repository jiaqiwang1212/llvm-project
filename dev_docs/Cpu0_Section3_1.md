# Cpu0 LLVM Backend — Section 3.1: TargetMachine Structure

## Purpose

Section 2.4.3 registered Cpu0 into LLVM's target registry and build system, but the
`TargetMachine` was an empty shell — no `Subtarget`, no `RegisterInfo`, no `InstrInfo`,
no `FrameLowering`. `llc -march=cpu0` could find the target, but entering the codegen
pipeline caused an immediate crash.

**Section 3.1 goal**: establish `Cpu0TargetMachine` and the full C++ backend class
hierarchy it aggregates, so that Cpu0 can run through a minimal codegen pipeline.

---

## Files Modified

| File | Change | Purpose |
|---|---|---|
| `Cpu0Subtarget.cpp/h` | new | Aggregates all backend components; parses CPU feature strings |
| `Cpu0RegisterInfo.cpp/h` | new | Callee-saved set, reserved registers, frame index elimination |
| `Cpu0SERegisterInfo.cpp/h` | new | SE-variant RegisterInfo (inherits base) |
| `Cpu0InstrInfo.cpp/h` | new | Instruction interface skeleton (copyPhysReg, etc.) |
| `Cpu0SEInstrInfo.cpp/h` | new | SE-variant InstrInfo |
| `Cpu0ISelLowering.cpp/h` | new | LowerFormalArguments / LowerReturn; custom DAG node types |
| `Cpu0SEISelLowering.cpp/h` | new | SE-variant ISelLowering |
| `Cpu0ISelDAGToDAG.cpp/h` | new | DAG-to-DAG instruction selector; SelectAddr ComplexPattern |
| `Cpu0FrameLowering.cpp/h` | new | Prologue/epilogue framework (skeleton; body filled in later) |
| `Cpu0SEFrameLowering.cpp/h` | new | SE-variant FrameLowering |
| `Cpu0MachineFunction.cpp/h` | new | Per-function backend private data container |
| `Cpu0TargetObjectFile.cpp/h` | new | ELF section assignment (inherits default ELF rules) |
| `MCTargetDesc/Cpu0ABIInfo.cpp/h` | new | O32 calling convention ABI decisions; argument register list |
| `MCTargetDesc/Cpu0MCAsmInfo.cpp/h` | new | ELF assembly format descriptor; fixes `-mcpu=help` segfault |
| `Cpu0TargetMachine.cpp/h` | modified | Wire in Subtarget; implement `getSubtargetImpl()`; register ISelDAGToDAG pass |
| `Cpu0.h` | modified | Add `createCpu0ISelDag()` and pass init forward declarations |
| `Cpu0CallingConv.td` | modified | Add `CC_Cpu0_I32` integer argument allocation rules |
| `CMakeLists.txt` | modified | Add new .cpp files to `LLVMCpu0CodeGen` |
| `MCTargetDesc/CMakeLists.txt` | modified | Add ABIInfo / MCAsmInfo to `LLVMCpu0Desc` |
| `MCTargetDesc/Cpu0MCTargetDesc.cpp` | modified | Include TableGen .inc files (MC_DESC guard variants); register MCAsmInfo |

---

## Key Design Decisions

### base / SE split pattern

`RegisterInfo`, `InstrInfo`, `FrameLowering`, and `ISelLowering` are each split into a
base class and an SE subclass. This mirrors the MIPS backend: the base class holds
shared logic, the SE subclass holds the standard implementation. Future variants (e.g.
Cpu0S) add a new subclass without touching the base.

### Subtarget as aggregator

`Cpu0Subtarget` holds concrete instances of the SE-series components and exposes them
through base-class pointers. It is the single entry point through which `TargetMachine`
retrieves all runtime backend objects.

### Wiring ISelDAGToDAG

`Cpu0PassConfig::addInstSelector()` calls `createCpu0ISelDag()` to register the
DAG-to-DAG selector pass. `SelectAddr` must be declared in the header because
`Cpu0GenDAGISel.inc` (TableGen-generated) references it directly in the class body.

### Why MCAsmInfo fixes the segfault

Tool paths such as `-mcpu=help` access `MCAsmInfo` before constructing
`MCSubtargetInfo`. Without registration the pointer is null, causing a segfault.
`Cpu0MCAsmInfo` inherits `MCAsmInfoELF` and is registered via `RegisterMCAsmInfo`
inside `LLVMInitializeCpu0TargetMC()`.

---

## How to Test

```bash
# 1. Build the three Cpu0 libraries
cmake --build . --target LLVMCpu0CodeGen LLVMCpu0Desc LLVMCpu0Info -- -j$(nproc)

# 2. Confirm llc lists Cpu0 targets
./bin/llc --version | grep -i cpu0
# Expected: cpu0 and cpu0el appear under "Registered Targets"

# 3. -mcpu=help must not crash
./bin/llc -march=cpu0 -mcpu=help 2>&1 | head -5
# Expected: prints available CPUs (cpu032I / cpu032II), no segfault

# 4. Minimal function compiles to assembly
echo 'define i32 @f() { ret i32 0 }' > /tmp/t.ll
./bin/llc -march=cpu0 -mcpu=cpu032I -relocation-model=pic /tmp/t.ll -o /tmp/t.s
cat /tmp/t.s
# Expected: valid Cpu0 assembly output, no crash
```

---

## State After This Section

| Capability | Status |
|---|---|
| `TargetMachine` holds a real `Subtarget` | ✅ |
| MC layer fully initialized; `MCAsmInfo` registered | ✅ |
| `ISelDAGToDAG` pass wired into the codegen pipeline | ✅ |
| `LowerFormalArguments` / `LowerReturn` driven by `CC_Cpu0_I32` | ✅ |
| Frame index elimination framework in place | ✅ |
| Minimal `ret i32 0` function compiles to assembly | ✅ |
| Full ISA (arithmetic, load/store, branches, calls) | ❌ next chapter |
| Prologue / epilogue code generation | ❌ framework ready, body pending |
| AsmPrinter / object file emission | ❌ next chapter |
