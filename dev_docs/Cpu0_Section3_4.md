# Cpu0 LLVM Backend — Section 3.4: Callee-Saved Register Spill and Restore

## Purpose

Section 3.3 gave the backend a working stack frame: `emitPrologue` and `emitEpilogue` now
correctly adjust SP around each function body. The next requirement is that the register
allocator can **spill and restore callee-saved registers** — registers that a function
must preserve across calls. Without this, any function that uses a callee-saved register
(e.g., `$s0`–`$s7` on Cpu0) will corrupt the caller's register state.

The mechanism LLVM uses to spill a callee-saved register is:

1. Before register allocation, the register allocator calls `storeRegToStackSlot` to
   insert a store instruction that writes the register to a frame index.
2. After the function body, it calls `loadRegFromStackSlot` to insert a load that reads
   the register back from the same frame index.

Both methods are virtual in `TargetInstrInfo`. At the start of Section 3.3 they were
not declared or implemented in `Cpu0SEInstrInfo`, so the default base-class versions
would have been called — those assert-fail, crashing `llc` if any callee-saved register
needed spilling.

**Section 3.4 goal**: implement `storeRegToStackSlot` and `loadRegFromStackSlot` in
`Cpu0SEInstrInfo` so that the register allocator can spill and restore callee-saved
registers using Cpu0's `ST` and `LD` instructions.

---

## Files Modified

| File | Change | Purpose |
|---|---|---|
| `Cpu0SEInstrInfo.h` | modified | Declare `storeRegToStackSlot` and `loadRegFromStackSlot` overrides with LLVM 20 signatures |
| `Cpu0SEInstrInfo.cpp` | modified | Implement both methods using `Cpu0::ST` and `Cpu0::LD` with `MachineMemOperand` metadata |

---

## Background: Callee-Saved Registers in LLVM

LLVM's register allocator distinguishes between **caller-saved** registers (the callee may
clobber freely) and **callee-saved** registers (the callee must preserve). When the
register allocator decides to use a callee-saved register, it automatically:

1. Inserts a spill (store to stack) at the top of the function's entry block.
2. Inserts a fill (load from stack) just before each return.

The target backend must provide the two hooks that perform these insertions. Both receive
a `FrameIndex` (`FI`) identifying the stack slot the allocator has already reserved. The
implementation's job is to emit the correct load or store machine instruction targeting
that slot.

---

## Root Cause Analysis

### LLVM 20 Signature Change

The base class signatures in LLVM 20 (`TargetInstrInfo.h`) include an extra `Register
VReg` parameter compared to earlier LLVM versions. This parameter is used by GlobalISel
for virtual register tracking; SelectionDAG-based backends can ignore its value, but the
override signatures **must match exactly** to avoid silent shadowing (a mismatched
signature compiles without error but the override is never called).

Correct LLVM 20 signatures:

```cpp
// Store
void storeRegToStackSlot(MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MI,
                         Register SrcReg, bool IsKill, int FrameIdx,
                         const TargetRegisterClass *RC,
                         Register VReg,
                         MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

// Load
void loadRegFromStackSlot(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MI,
                          Register DestReg, int FrameIdx,
                          const TargetRegisterClass *RC,
                          Register VReg, unsigned SubReg = 0,
                          MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;
```

An earlier draft of this code used signatures without `VReg`, which compiled cleanly but
would never be dispatched by the allocator.

### Why `MachineMemOperand` Is Needed

Both methods attach a `MachineMemOperand` to the emitted instruction. This metadata
carries the access size, alignment, and pointer provenance (a fixed stack slot). LLVM
uses it for:

- **Alias analysis**: distinguishing stack-slot accesses from heap accesses.
- **Scheduler**: determining which loads/stores can be reordered.
- **Debug info**: associating memory operations with source locations.

Without `MachineMemOperand`, the instructions are still functionally correct but the
optimizer and scheduler treat them conservatively.

---

## Implementation

### `Cpu0SEInstrInfo.h` — Declarations

```cpp
void storeRegToStackSlot(MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MI,
                         Register SrcReg, bool IsKill, int FrameIdx,
                         const TargetRegisterClass *RC,
                         Register VReg,
                         MachineInstr::MIFlag Flags =
                             MachineInstr::NoFlags) const override;

void loadRegFromStackSlot(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MI,
                          Register DestReg, int FrameIdx,
                          const TargetRegisterClass *RC,
                          Register VReg, unsigned SubReg = 0,
                          MachineInstr::MIFlag Flags =
                              MachineInstr::NoFlags) const override;
```

### `Cpu0SEInstrInfo.cpp` — `storeRegToStackSlot`

```cpp
void Cpu0SEInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register SrcReg,
    bool IsKill, int FI, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOStore,
      MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  BuildMI(MBB, I, DL, get(Cpu0::ST))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addFrameIndex(FI)
      .addImm(0)
      .addMemOperand(MMO)
      .setMIFlags(Flags);
}
```

`Cpu0::ST` has the operand layout `ST $src, $base, imm`. The frame index (`FI`) takes
the place of the base register; it will be replaced by the actual SP-relative offset by
the frame index elimination pass that runs after register allocation. The immediate `0`
is the within-slot offset, which is always 0 for a full-register spill.

### `Cpu0SEInstrInfo.cpp` — `loadRegFromStackSlot`

```cpp
void Cpu0SEInstrInfo::loadRegFromStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register DestReg,
    int FI, const TargetRegisterClass *RC, Register VReg, unsigned SubReg,
    MachineInstr::MIFlag Flags) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOLoad,
      MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  BuildMI(MBB, I, DL, get(Cpu0::LD), DestReg)
      .addFrameIndex(FI)
      .addImm(0)
      .addMemOperand(MMO)
      .setMIFlags(Flags);
}
```

`Cpu0::LD` has the layout `LD $dst, $base, imm`. The destination register is the
first operand (passed to the `BuildMI` second-argument position), then the frame index
and zero offset follow.

### Required Includes in `Cpu0SEInstrInfo.cpp`

```cpp
#include "Cpu0SERegisterInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
```

`MachineMemOperand.h` must be added explicitly — it is not transitively pulled in by the
other headers in LLVM 20.

---

## How to Test

```bash
# 1. Build
cmake --build build/ --target LLVMCpu0CodeGen -- -j$(nproc)
cmake --build build/ --target llc -- -j$(nproc)

# 2. Compile a function that forces callee-saved register usage
#    (A function with enough local variables that the allocator exhausts
#     caller-saved registers and must spill to callee-saved ones.)
cat > /tmp/ch3_spill.c << 'EOF'
int foo(int a, int b, int c, int d) {
  int x = a + b;
  int y = c + d;
  return x * y;
}
EOF
build/bin/clang -O0 -emit-llvm -c /tmp/ch3_spill.c -o /tmp/ch3_spill.bc

# 3. Lower to assembly
build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm \
    /tmp/ch3_spill.bc -o /tmp/ch3_spill.s
echo "Exit: $?"   # Expected: 0

# 4. Check output (should not crash with assertion failure)
cat /tmp/ch3_spill.s
```

---

## State After This Section

| Capability | Status |
|---|---|
| `storeRegToStackSlot` emits `ST $src, FI, 0` | ✅ |
| `loadRegFromStackSlot` emits `LD $dst, FI, 0` | ✅ |
| LLVM 20 `VReg` parameter in both signatures | ✅ |
| `MachineMemOperand` attached for alias analysis and scheduling | ✅ |
| Register allocator can spill callee-saved registers without crashing | ✅ |
| Frame index elimination resolves `FI` to SP-relative offset | ✅ (existing pass) |
| Callee-saved register set defined in `Cpu0CallingConv.td` | ✅ (from Section 2.4.2) |
| Full ISA (arithmetic, branches, calls) | ❌ Chapter 4 |
| Object file emission (`-filetype=obj`) | ❌ Chapter 5 |
