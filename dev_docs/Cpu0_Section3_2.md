# Cpu0 LLVM Backend — Section 3.2: AsmPrinter and Function Return

## Purpose

Section 3.1 established the full backend class hierarchy and wired the codegen pipeline.
Running `llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc` still failed with a
cascade of three errors — each one exposed only after the previous was fixed:

1. **"Cannot select: Cpu0ISD::Ret"** — The SelectionDAG had no TableGen pattern to match
   the `Cpu0ISD::Ret` node to a machine instruction.
2. **"Cannot select: i32 = FrameIndex<0>"** — `SelectAddr` did not convert
   `FrameIndexSDNode` to a `TargetFrameIndex`, so the DAG legalizer could not lower frame
   accesses.
3. **SIGSEGV in `Cpu0AsmPrinter::emitFunctionBodyStart`** — `Cpu0FI` (the per-function
   info pointer) was null. In LLVM 20 `MachineFunction::getInfo<T>()` does **not**
   auto-create `T`; the target must override `TargetMachine::createMachineFunctionInfo`.

**Section 3.2 goal**: fix all three failures so that `llc` exits 0 and emits valid Cpu0
assembly containing a `ret` instruction.

---

## Files Modified

| File | Change | Purpose |
|---|---|---|
| `Cpu0InstrInfo.td` | modified | Define `SDT_Cpu0Ret` type profile; add `Pat<>` to match `Cpu0ISD::Ret` to the `RET` machine instruction |
| `Cpu0ISelLowering.cpp` | modified | Fix `LowerReturn`: stop pushing return-value registers into `RetOps`; only push `LR` and glue |
| `Cpu0ISelDAGToDAG.cpp` | modified | Fix `SelectAddr`: handle `FrameIndexSDNode` by converting to `TargetFrameIndex` with offset 0 |
| `Cpu0MachineFunction.h` | modified | Update `Cpu0FunctionInfo` constructor to LLVM 20 `MachineFunctionInfo` factory API signature |
| `Cpu0TargetMachine.h` | modified | Declare `createMachineFunctionInfo` override |
| `Cpu0TargetMachine.cpp` | modified | Implement `createMachineFunctionInfo` using the `MachineFunctionInfo::create<T>` factory |
| `Cpu0AsmPrinter.cpp` | modified | Add `Cpu0AsmPrinter` and `emitFunctionBodyStart`/`emitFunctionBodyEnd` implementation (works now because `Cpu0FI` is guaranteed non-null) |

---

## Root Cause Analysis

### Fix 1 — "Cannot select: Cpu0ISD::Ret"

The `Cpu0Ret` SDNode used `SDTNone` as its type profile, meaning it declared zero operands.
The `RET` machine instruction expects a register operand (`$lr`). TableGen could not
generate a match because the node and the instruction had incompatible operand signatures.

Two changes together resolved this:

**`Cpu0InstrInfo.td`** — `SDT_Cpu0Ret` type profile and a `Pat<>` rule:

```tablegen
def SDT_Cpu0Ret : SDTypeProfile<0, 1, [SDTCisInt<0>]>;

def Cpu0Ret : SDNode<"Cpu0ISD::Ret", SDT_Cpu0Ret,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
...
def : Pat<(Cpu0Ret GPROut:$ra), (RET GPROut:$ra)>;
```

**`Cpu0ISelLowering.cpp`** — `LowerReturn` was also pushing the return-value register
(e.g. `$v0`) directly into `RetOps`. That put two registers in the operand list but the
type profile only declared one. The fix removes that push; only `LR` and the glue token
belong in `RetOps`:

```cpp
for (unsigned I = 0; I < RVLocs.size(); ++I) {
    const CCValAssign &VA = RVLocs[I];
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[I], Glue);
    Glue = Chain.getValue(1);
    // return-value register is NOT pushed into RetOps here
}
RetOps[0] = Chain;
RetOps.push_back(DAG.getRegister(Cpu0::LR, MVT::i32));
if (Glue.getNode())
    RetOps.push_back(Glue);
return DAG.getNode(Cpu0ISD::Ret, DL, MVT::Other, RetOps);
```

---

### Fix 2 — "Cannot select: i32 = FrameIndex"

`SelectAddr` fell through to the generic `Base = N` path for every input, including
`FrameIndexSDNode`. A `FrameIndex` is a pre-RA placeholder; it must become a
`TargetFrameIndex` before the instruction selector can emit a load/store. Without the
conversion, the DAG legalizer had no pattern to match the raw frame index node.

**`Cpu0ISelDAGToDAG.cpp`**:

```cpp
bool Cpu0DAGToDAGISel::SelectAddr(SDValue N, SDValue &Base, SDValue &Offset) {
    EVT ValTy = N.getValueType();
    SDLoc DL(N);
    if (FrameIndexSDNode *FI = dyn_cast<FrameIndexSDNode>(N)) {
        Base   = CurDAG->getTargetFrameIndex(FI->getIndex(), ValTy);
        Offset = CurDAG->getTargetConstant(0, DL, ValTy);
        return true;
    }
    Base   = N;
    Offset = CurDAG->getTargetConstant(0, DL, ValTy);
    return true;
}
```

---

### Fix 3 — SIGSEGV in `emitFunctionBodyStart`

`Cpu0AsmPrinter::runOnMachineFunction` calls `MF.getInfo<Cpu0FunctionInfo>()` and stores
the result in `Cpu0FI`. In LLVM 20 this function does an **unchecked `static_cast`** — it
does not construct the object if it doesn't exist. The pointer was null because no one
had ever allocated a `Cpu0FunctionInfo` for the function.

The fix involves two files:

**`Cpu0MachineFunction.h`** — update the constructor to the LLVM 20 factory signature:

```cpp
explicit Cpu0FunctionInfo(const Function &F, const TargetSubtargetInfo *STI) {}
```

**`Cpu0TargetMachine.h`** — declare the override:

```cpp
MachineFunctionInfo *
createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                          const TargetSubtargetInfo *STI) const override;
```

**`Cpu0TargetMachine.cpp`** — implement it using the `MachineFunctionInfo::create<T>`
placement-new factory (LLVM allocates from a `BumpPtrAllocator` for performance):

```cpp
MachineFunctionInfo *
Cpu0TargetMachine::createMachineFunctionInfo(BumpPtrAllocator &Allocator,
                                              const Function &F,
                                              const TargetSubtargetInfo *STI) const {
    return Cpu0FunctionInfo::create<Cpu0FunctionInfo>(Allocator, F, STI);
}
```

With this in place, LLVM calls `createMachineFunctionInfo` when constructing each
`MachineFunction`, so `Cpu0FI` is always non-null by the time `emitFunctionBodyStart`
is reached.

---

## How to Test

```bash
# 1. Build (from the cmake build directory, e.g. build/)
cmake --build . --target LLVMCpu0CodeGen -- -j$(nproc)
# Rebuild llc
cmake --build . --target llc -- -j$(nproc)

# 2. Compile ch3.bc to Cpu0 assembly
./bin/llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc -o /tmp/ch3.cpu0.s
echo "Exit: $?"   # Expected: 0

# 3. Verify ret instruction is present
grep 'ret' /tmp/ch3.cpu0.s
# Expected: ret $lr

# 4. Inspect the full output
cat /tmp/ch3.cpu0.s
# Expected output (roughly):
#   main:
#       .frame  $fp,8,$lr
#       .mask   0x00000000,0
#       .set    noreorder
#       .set    nomacro
#   # %bb.0:
#       addiu   $2, $zero, 0
#       st      $2, 0($fp)
#       ret     $lr
#       .set    macro
#       .set    reorder
#       .end    main
```

---

## State After This Section

| Capability | Status |
|---|---|
| `Cpu0ISD::Ret` selects to `RET $lr` | ✅ |
| `FrameIndex` nodes lowered to `TargetFrameIndex` in `SelectAddr` | ✅ |
| `Cpu0FunctionInfo` allocated via `createMachineFunctionInfo` (LLVM 20 API) | ✅ |
| `Cpu0AsmPrinter` emits `.frame` / `.mask` / `.set` directives | ✅ |
| `llc` exits 0 for a minimal `int main() { return 0; }` input | ✅ |
| Full ISA (arithmetic, load/store, branches, calls) | ❌ next chapter |
| Prologue / epilogue with non-trivial stack frames | ❌ next chapter |
| Object file emission (`-filetype=obj`) | ❌ next chapter |
