# Cpu0 LLVM Backend — Section 3.3: Stack Frame Prologue and Epilogue

## Purpose

Section 3.2 brought `llc` to the point where it exits 0 and emits a valid `ret` instruction
for a minimal `int main() { return 0; }`. However, `emitPrologue` and `emitEpilogue` in
`Cpu0SEFrameLowering` were left as empty stubs. This means:

- No stack space is allocated when entering a function.
- No stack space is released when leaving a function.
- Local variables that the register allocator spills to the stack will read and write at
  wrong addresses, producing silent memory corruption.

**Section 3.3 goal**: implement `emitPrologue` and `emitEpilogue` so that `llc` emits
correct `ADDiu $sp, $sp, -N` / `ADDiu $sp, $sp, +N` pairs around each function body.

---

## Files Modified

| File | Change | Purpose |
|---|---|---|
| `Cpu0SEFrameLowering.cpp` | modified | Implement `emitPrologue` and `emitEpilogue` with chunked `ADDiu` loops |

---

## Background: Why the Stack Frame Matters Here

After Section 3.2, `llc` produces assembly for a zero-local-variable function. The output
contains `.frame $fp,8,$lr` but no actual SP adjustment instruction — the stub
`emitPrologue` emits nothing. For a leaf function that uses no stack slots this is
accidentally correct. But as soon as the function has any local variable (e.g.,
`int x = 5;`), the register allocator creates a frame index for `x`, the frame offset
calculation assumes SP was adjusted by `StackSize` bytes, and the resulting `st`/`ld`
instructions reference the wrong address.

The fix is straightforward: emit the arithmetic instruction that adjusts SP at function
entry (prologue) and restore SP at function exit (epilogue).

---

## Root Cause Analysis

### Why `emitPrologue` Was Empty in 3.1

`Cpu0SEFrameLowering` was scaffolded in Section 3.1 as a class skeleton.  Its two key
virtual methods inherited from `TargetFrameLowering` — `emitPrologue` and `emitEpilogue`
— were declared with empty bodies. This was intentional: 3.1 targeted the absolute minimum
required to get the codegen pipeline to link and not crash. Stack frame adjustment was
deferred to 3.3.

### The Chunking Constraint

Cpu0's `ADDiu` instruction encodes its immediate operand in a 16-bit signed field, giving
a range of −32768 to +32767. A single `ADDiu` can therefore adjust SP by at most 32767
bytes in one direction. For functions with a frame larger than that limit — e.g., a
function with a large local array — a single `ADDiu` cannot represent the full adjustment.

The standard production solution (used by MIPS and others) is to emit a `LUi + ORi +
ADDU` sequence to build an arbitrary 32-bit constant and add it to SP. However, the Cpu0
ISA as defined in the tutorial **does not include `LUi`** at this stage. The workaround is
to loop, emitting multiple `ADDiu` instructions each covering at most 32767 bytes:

```
frame size = 100000 bytes  →  ADDiu $sp, $sp, -32767
                               ADDiu $sp, $sp, -32767
                               ADDiu $sp, $sp, -32767
                               ADDiu $sp, $sp, -1699   (100000 - 3×32767)
```

This is correct but not optimal; Chapter 4 will add the full ISA and a proper large-frame
sequence can be introduced then.

---

## Implementation

### `emitPrologue`

```cpp
void Cpu0SEFrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const Cpu0InstrInfo &TII =
      *static_cast<const Cpu0InstrInfo *>(MF.getSubtarget().getInstrInfo());

  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  uint64_t StackSize = MFI.getStackSize();

  if (StackSize == 0 && !MFI.adjustsStack())
    return;

  // Emit ADDiu in 32767-byte chunks to support large frames (> 16-bit limit).
  uint64_t Remaining = StackSize;
  while (Remaining > 0) {
    uint64_t Chunk = Remaining > 32767 ? 32767 : Remaining;
    BuildMI(MBB, MBBI, DL, TII.get(Cpu0::ADDiu), Cpu0::SP)
        .addReg(Cpu0::SP)
        .addImm(-(int64_t)Chunk);
    Remaining -= Chunk;
  }
}
```

Key points:
- The early return on `StackSize == 0 && !MFI.adjustsStack()` skips the prologue entirely
  for leaf functions that make no stack adjustments — this keeps the output clean for
  trivial functions.
- `MBB.begin()` positions the new instructions at the very top of the entry block, before
  any user code.
- The negative immediate decrements SP (stack grows downward on Cpu0).

### `emitEpilogue`

```cpp
void Cpu0SEFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const Cpu0InstrInfo &TII =
      *static_cast<const Cpu0InstrInfo *>(MF.getSubtarget().getInstrInfo());

  DebugLoc DL = MBBI->getDebugLoc();
  uint64_t StackSize = MFI.getStackSize();

  if (StackSize == 0)
    return;

  // Emit ADDiu in 32767-byte chunks to support large frames (> 16-bit limit).
  uint64_t Remaining = StackSize;
  while (Remaining > 0) {
    uint64_t Chunk = Remaining > 32767 ? 32767 : Remaining;
    BuildMI(MBB, MBBI, DL, TII.get(Cpu0::ADDiu), Cpu0::SP)
        .addReg(Cpu0::SP)
        .addImm((int64_t)Chunk);
    Remaining -= Chunk;
  }
}
```

Key points:
- `getLastNonDebugInstr()` returns an iterator pointing to the `ret` instruction; the
  `BuildMI` calls insert the SP restoration *before* it, which is the correct position.
- The positive immediate increments SP, restoring the caller's stack pointer.
- The epilogue does not need the `adjustsStack()` guard because a zero `StackSize`
  means nothing was allocated in the prologue — there is nothing to restore.

---

## How to Test

```bash
# 1. Build the Cpu0 CodeGen library and llc
cmake --build build/ --target LLVMCpu0CodeGen -- -j$(nproc)
cmake --build build/ --target llc -- -j$(nproc)

# 2. Compile a function with a local variable
cat > /tmp/ch3_local.c << 'EOF'
int main() {
  volatile int x = 5;
  return x;
}
EOF
build/bin/clang -O0 -emit-llvm -c /tmp/ch3_local.c -o /tmp/ch3_local.bc

# 3. Lower to Cpu0 assembly
build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm \
    /tmp/ch3_local.bc -o /tmp/ch3_local.s
echo "Exit: $?"   # Expected: 0

# 4. Verify SP adjustment instructions are present
grep 'addiu.*sp' /tmp/ch3_local.s
# Expected: at least one negative addiu (prologue) and one positive addiu (epilogue)

# 5. Inspect full output
cat /tmp/ch3_local.s
# Expected (approximately):
#   main:
#       .frame  $fp,8,$lr
#       addiu   $sp, $sp, -8    # prologue: allocate 8-byte frame
#       ...
#       addiu   $sp, $sp, 8     # epilogue: release frame
#       ret     $lr
```

---

## State After This Section

| Capability | Status |
|---|---|
| `emitPrologue` adjusts SP on function entry | ✅ |
| `emitEpilogue` restores SP on function exit | ✅ |
| Large frames (>32767 bytes) handled via chunked `ADDiu` loop | ✅ |
| Optimal large-frame sequence (`LUi` + `ORi` + `ADDU`) | ❌ ISA lacks `LUi` until Chapter 4 |
| Callee-saved register spill / restore | ❌ next section (3.4) |
| Full ISA (arithmetic, branches, calls) | ❌ Chapter 4 |
