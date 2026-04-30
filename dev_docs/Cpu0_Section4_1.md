# Cpu0 LLVM Backend — Section 4.1: Arithmetic Instructions

## Purpose

Chapter 4 extends the Cpu0 backend from "barely runs" to a real arithmetic compiler.
Section 4.1 adds **every arithmetic, shift, rotate, multiply, and divide instruction** the
Cpu0 ISA defines, together with the HILO register pair, an `EnableOverflow` option, and
the DAG-combine plumbing that lets LLVM lower signed/unsigned division to the two-
instruction `div` + `mfhi`/`mflo` sequence.

After this section the backend can compile complete C arithmetic expressions (including
`%` and `/` with variable divisors) to correct Cpu0 assembly.

**Also included:** the Chapter 3.5 / 3.6 instruction infrastructure that was deferred from
earlier sections — `ArithLogicR`, `shift_rotate_imm32`, `LUi`, `ORi`, `ADDu`, `SHL`, and
the `HI16` / `LO16` DAG transforms for 32-bit immediate synthesis — because Chapter 4.1
depends on all of it.

---

## Files Modified

| File | Change |
|---|---|
| `Cpu0RegisterInfo.td` | Add `HI`, `LO` registers and `HILO` register class |
| `Cpu0Schedule.td` | Add `IIHiLo`, `IImul`, `IIIdiv` itinerary classes; update `Cpu0GenericItineraries` |
| `Cpu0InstrInfo.td` | Add SDNodes, operand predicates, instruction classes, and ~25 new instruction defs |
| `Cpu0Subtarget.cpp` | Add `EnableOverflowOpt` command-line option |
| `Cpu0SEISelLowering.h` | Declare `PerformDAGCombine` override (SE subclass, not base) |
| `Cpu0SEISelLowering.cpp` | Expand SDIV/SREM/UDIV/UREM; mark MULHS/MULHU Legal; add `performDivRemCombine` + `PerformDAGCombine` |
| `Cpu0SEInstrInfo.h` | Declare `copyPhysReg` override |
| `Cpu0SEInstrInfo.cpp` | Implement `copyPhysReg` for GPR ↔ HI/LO register moves |
| `Cpu0ISelDAGToDAG.h` | Declare `selectMULT` |
| `Cpu0ISelDAGToDAG.cpp` | Implement `selectMULT`; handle `ISD::MULHS` / `ISD::MULHU` in `Select()` |

---

## Change Plan

### 1 · Cpu0RegisterInfo.td — HI/LO registers

**Why:** The multiply instructions (`MULT`, `MULTu`) and divide instructions (`SDIV`,
`UDIV`) write their results into special `HI` and `LO` registers (high word / low word).
`MFHI`/`MFLO` then copy from them to GPRs. LLVM's register allocator needs an explicit
register class to model these.

**What to add** — inside `let Namespace = "Cpu0" in {}`:

```tablegen
  // HI/LO registers for multiply/divide results
  def HI : Cpu0Reg<0, "hi">, DwarfRegNum<[18]>;
  def LO : Cpu0Reg<1, "lo">, DwarfRegNum<[19]>;
```

> **Implementation note:** The tutorial uses `"ac0"` as the assembly name for both HI and
> LO with encoding 0. The actual implementation uses `"hi"`/`"lo"` as distinct names and
> encoding 0/1 respectively.

**What to add** — after `C0Regs`:

```tablegen
// HI/LO register class for multiply/divide results
def HILO : RegisterClass<"Cpu0", [i32], 32, (add HI, LO)>;
```

---

### 2 · Cpu0Schedule.td — multiply/divide itineraries

**Why:** LLVM's instruction scheduler uses itinerary data to model latency. Multiply takes
17 cycles and divide 38 cycles on the IMULDIV functional unit; `MFHI`/`MFLO` take 1.

**What to add** — new `InstrItinClass` defs:

```tablegen
def IIHiLo  : InstrItinClass;
def IImul   : InstrItinClass;
def IIIdiv  : InstrItinClass;
```

**Update `Cpu0GenericItineraries`** — append inside the existing list:

```tablegen
  InstrItinData<IIHiLo, [InstrStage<1,  [IMULDIV]>]>,
  InstrItinData<IImul,  [InstrStage<17, [IMULDIV]>]>,
  InstrItinData<IIIdiv, [InstrStage<38, [IMULDIV]>]>,
```

---

### 3 · Cpu0InstrInfo.td — SDNodes, operands, classes, and instruction defs

This is the largest change. It is split into logical subsections.

#### 3a · New SDNodes (at top of file, near `SDT_Cpu0Ret`)

The division DAG combine produces two custom nodes for signed/unsigned `divrem`:

```tablegen
// Only op DAG can be disabled by ch4_1, data DAG cannot.
def SDT_Cpu0DivRem : SDTypeProfile<0, 2,
                       [SDTCisInt<0>, SDTCisSameAs<0, 1>]>;

def Cpu0DivRem  : SDNode<"Cpu0ISD::DivRem",  SDT_Cpu0DivRem, [SDNPOutGlue]>;
def Cpu0DivRemU : SDNode<"Cpu0ISD::DivRemU", SDT_Cpu0DivRem, [SDNPOutGlue]>;
```

#### 3b · Immediate operand helpers (after existing `immSExt16`)

> **Implementation note:** The tutorial uses `getImm(N, v)` inside `SDNodeXForm` and
> defines `immZExt16` as a `PatLeaf` with explicit cast-comparison logic, and `immZExt5`
> as `ImmLeaf`. The actual implementation uses `CurDAG->getTargetConstant(...)` in XForms
> and simpler `isUInt<N>` checks. `immLow16Zero` checks `(Val & 0xffffULL) == 0` using
> `getZExtValue()` (unsigned) rather than `getSExtValue()` (signed).

```tablegen
// Immediate whose low 16 bits are zero (upper-half load pattern for LUi).
def immLow16Zero : PatLeaf<(imm), [{
  uint64_t Val = N->getZExtValue();
  return (Val & 0xffffULL) == 0;
}]>;

// Zero-extended 16-bit immediate (matches 0..65535).
def immZExt16 : PatLeaf<(imm), [{ return isUInt<16>(N->getZExtValue()); }]>;

// Unsigned 16-bit immediate operand (for encoding).
def uimm16 : Operand<i32> {
  let DecoderMethod = "DecodeSimm16";
}

// Unsigned 5-bit shift amount operand (for encoding).
def uimm5 : Operand<i32> {
  let DecoderMethod = "DecodeSimm16";
}

// PatLeaf matching a 5-bit unsigned shift amount.
def immZExt5 : PatLeaf<(imm), [{ return isUInt<5>(N->getZExtValue()); }]>;

// Extract upper/lower 16 bits from a 32-bit immediate constant.
def HI16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((N->getZExtValue() >> 16) & 0xffff,
                                    SDLoc(N), MVT::i32);
}]>;

def LO16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() & 0xffff,
                                    SDLoc(N), MVT::i32);
}]>;
```

#### 3c · ArithLogicR instruction class (FA-format, 3-register)

```tablegen
class ArithLogicR<bits<8> op, string instr_asm, SDNode OpNode,
                  InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
  let shamt = 0;
  let isCommutable = isComm;
  let isReMaterializable = 1;
}
```

#### 3d · Shift / rotate instruction classes

> **Implementation note:** The tutorial names these `shift_rotate_imm32` and
> `shift_rotate_reg`. The actual implementation uses `ShiftRotateI` and `ShiftRotateR`
> (cleaner naming). The `isRotate` parameter is dropped as it is unused at the class level.

```tablegen
// Immediate shift amount (e.g. SHL, SHR, SRA, ROL, ROR)
class ShiftRotateI<bits<8> op, string instr_asm,
                   SDNode OpNode, PatLeaf imm_type, RegisterClass RC> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, uimm5:$shamt),
     !strconcat(instr_asm, "\t$ra, $rb, $shamt"),
     [(set GPROut:$ra, (OpNode RC:$rb, imm_type:$shamt))], IIAlu> {
  let rc = 0;
}

// Register shift amount (e.g. SHLV, SHRV, SRAV, ROLV, RORV)
class ShiftRotateR<bits<8> op, string instr_asm, SDNode OpNode,
                   RegisterClass RC> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (OpNode RC:$rb, RC:$rc))], IIAlu> {
  let shamt = 0;
}
```

#### 3e · Multiply/Divide instruction classes

> **Implementation note:** The tutorial defines a generic `Mult` base class and a `Div`
> base class parameterized over register class. The actual implementation collapses each
> to a single concrete `Mult32` / `Div32` class (always using `CPURegs` and `[HI, LO]`),
> removing the indirection.  The `MoveToC0` input operand is named `$rb` (not `$ra`) to
> match the FA format's `rb` field. `C0Move` outputs to `$rb` and takes `$ra` as input.

```tablegen
// Multiply: results go to HI/LO implicitly.
class Mult32<bits<8> op, string instr_asm, InstrItinClass itin> :
  FA<op, (outs), (ins CPURegs:$ra, CPURegs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], itin> {
  let rc = 0;
  let shamt = 0;
  let isCommutable = 1;
  let Defs = [HI, LO];
}

// Divide: results go to HI/LO implicitly; SDNode pattern for isel.
class Div32<SDNode opNode, bits<8> op, string instr_asm, InstrItinClass itin> :
  FA<op, (outs), (ins CPURegs:$ra, CPURegs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(opNode CPURegs:$ra, CPURegs:$rb)], itin> {
  let rc = 0;
  let shamt = 0;
  let Defs = [HI, LO];
}

// Move from HI or LO register
class MoveFromLOHI<bits<8> op, string instr_asm, RegisterClass RC,
                   list<Register> UseRegs> :
  FA<op, (outs RC:$ra), (ins),
     !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let rc = 0;
  let shamt = 0;
  let Uses = UseRegs;
}

// Move to HI or LO register
class MoveToLOHI<bits<8> op, string instr_asm, RegisterClass RC,
                 list<Register> DefRegs> :
  FA<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let rc = 0;
  let shamt = 0;
  let Defs = DefRegs;
}

// Move to C0 register (input operand named $rb to match FA encoding field)
class MoveToC0<bits<8> op, string instr_asm, RegisterClass RC> :
  FA<op, (outs C0Regs:$ra), (ins RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], IIAlu> {
  let rc = 0;
  let shamt = 0;
}

// Move from C0 register
class MoveFromC0<bits<8> op, string instr_asm, RegisterClass RC> :
  FA<op, (outs RC:$ra), (ins C0Regs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], IIAlu> {
  let rc = 0;
  let shamt = 0;
}

// Move between C0 registers (output $rb, input $ra)
class C0Move<bits<8> op, string instr_asm> :
  FA<op, (outs C0Regs:$rb), (ins C0Regs:$ra),
     !strconcat(instr_asm, "\t$rb, $ra"), [], IIAlu> {
  let rc = 0;
  let shamt = 0;
}
```

#### 3f · New instruction definitions

**Ch3_5 prerequisites** (gated by `let Predicates = [Ch3_5]`):

```tablegen
// Logical immediate (OR immediate, zero-extended)
def ORi  : ArithLogicI<0x0d, "ori", or, uimm16, immZExt16, CPURegs>;

// Load Upper Immediate (FL format, no ISel pattern — matched via Pat rules below)
def LUi  : FL<0x0f, (outs GPROut:$ra), (ins uimm16:$imm16),
              "lui\t$ra, $imm16", [], IIHiLo> {
  let rb = 0;
}

// Arithmetic (unsigned) add — no overflow trap (also under DisableOverflow)
def ADDu : ArithLogicR<0x11, "addu", add, IIAlu, GPROut, 1>;

// Shift left logical (immediate amount)
def SHL  : ShiftRotateI<0x1e, "shl", shl, immZExt5, GPROut>;
```

**Ch4_1 instruction definitions** (all inside `let Predicates = [Ch4_1] in`):

| Def | Opcode | Class | Predicate guard | Description |
|---|---|---|---|---|
| `SUBu` | `0x12` | `ArithLogicR` | `DisableOverflow` | unsigned subtract |
| `ADD` | `0x13` | `ArithLogicR` | `EnableOverflow` | signed add — traps on overflow |
| `SUB` | `0x14` | `ArithLogicR` | `EnableOverflow` | signed subtract — traps on overflow |
| `MUL` | `0x17` | `ArithLogicR` | — | multiply (lo word result in GPR) |
| `AND` | `0x18` | `ArithLogicR` | — | bitwise AND |
| `OR` | `0x19` | `ArithLogicR` | — | bitwise OR |
| `XOR` | `0x1a` | `ArithLogicR` | — | bitwise XOR |
| `NOR` | `0x1b` | `FA` (inline) | — | bitwise NOR (no standard SDNode; matched via `Pat<(not (or ...))>`) |
| `ROL` | `0x1c` | `ShiftRotateI` | — | rotate left |
| `ROR` | `0x1d` | `ShiftRotateI` | — | rotate right |
| `SHR` | `0x1f` | `ShiftRotateI` | — | logical shift right |
| `SRA` | `0x20` | `ShiftRotateI` | — | arithmetic shift right |
| `SRAV` | `0x21` | `ShiftRotateR` | — | arithmetic shift right (reg amt) |
| `SHLV` | `0x22` | `ShiftRotateR` | — | shift left (reg amt) |
| `SHRV` | `0x23` | `ShiftRotateR` | — | logical shift right (reg amt) |
| `ROLV` | `0x24` | `ShiftRotateR` | — | rotate left (reg amt) |
| `RORV` | `0x25` | `ShiftRotateR` | — | rotate right (reg amt) |
| `MULT` | `0x41` | `Mult32` | — | signed 32×32→64 multiply (→HI:LO) |
| `MULTu` | `0x42` | `Mult32` | — | unsigned 32×32→64 multiply (→HI:LO) |
| `SDIV` | `0x43` | `Div32<Cpu0DivRem>` | — | signed divide |
| `UDIV` | `0x44` | `Div32<Cpu0DivRemU>` | — | unsigned divide |
| `MFHI` | `0x46` | `MoveFromLOHI` | — | move from HI |
| `MFLO` | `0x47` | `MoveFromLOHI` | — | move from LO |
| `MTHI` | `0x48` | `MoveToLOHI` | — | move to HI |
| `MTLO` | `0x49` | `MoveToLOHI` | — | move to LO |
| `SLT` / `SLTu` | `0x28`/`0x29` | `SetCC_R` | `HasSlt` | set-less-than (cpu032II) |
| `SLTi` / `SLTiu` | `0x2a`/`0x2b` | `SetCC_I` | `HasSlt` | set-less-than immediate |
| `CMP` | `0x2c` | `FA` (inline) | `HasCmp` | compare (cpu032I; no pattern) |
| `MFC0` | `0x50` | `MoveFromC0` | — | move from coprocessor-0 reg |
| `MTC0` | `0x51` | `MoveToC0` | — | move to coprocessor-0 reg |
| `C0MOV` | `0x52` | `C0Move` | — | move between C0 registers |

> **Implementation note:** `AND`, `OR`, `XOR`, `NOR`, `SLT`/`SLTu`/`SLTi`/`SLTiu`, and
> `CMP` are not mentioned in the tutorial's Chapter 4.1 instruction table but are added in
> the actual implementation to support later chapters. `SLT` variants use a `SetCC_R` /
> `SetCC_I` helper class; `CMP` is inlined as a raw `FA` with no SelectionDAG pattern.

**Large immediate synthesis patterns** (Ch3_5 predicate):

```tablegen
// Zero-extended 16-bit immediate -> ORi ZERO, imm
def : Pat<(i32 immZExt16:$in), (ORi ZERO, imm:$in)>;
// Upper-half load: immediate with low 16 bits zero -> LUi
def : Pat<(i32 immLow16Zero:$in), (LUi (HI16 imm:$in))>;
// Arbitrary 32-bit immediate -> LUi upper + ORi lower
def : Pat<(i32 imm:$imm), (ORi (LUi (HI16 imm:$imm)), (LO16 imm:$imm))>;
```

**NOR pattern** (Ch4_1 predicate):

```tablegen
def : Pat<(not (or GPROut:$rb, GPROut:$rc)), (NOR GPROut:$rb, GPROut:$rc)>;
```

---

### 4 · Cpu0Subtarget.cpp — EnableOverflowOpt option

**Why:** `ADD` and `SUB` trap on overflow; `ADDu` and `SUBu` silently wrap. In practice
almost all C code uses the non-trapping variants, but the trapping variants are useful for
debugging. A command-line flag lets the user opt in.

**What to add** — before the constructor, at file scope:

```cpp
static cl::opt<bool> EnableOverflowOpt(
    "cpu0-enable-overflow", cl::Hidden, cl::init(false),
    cl::desc("Use trigger overflow instructions add and sub "
             "instead of non-overflow instructions addu and subu"));
```

**What to add** inside `initializeSubtargetDependencies()`, before `ParseSubtargetFeatures`:

```cpp
EnableOverflow = EnableOverflowOpt;
```

**Required header:** `#include "llvm/Support/CommandLine.h"`

---

### 5 · Cpu0SEISelLowering.h — PerformDAGCombine declaration

**Why:** LLVM's `srem`/`urem` nodes expand to `sdivrem`/`udivrem` (via
`setOperationAction(..., Expand)`). Then `setTargetDAGCombine` fires our callback to
convert `sdivrem`/`udivrem` into the custom `Cpu0ISD::DivRem` node plus `CopyFromReg`
nodes for HI/LO.

> **Implementation note:** The tutorial places `PerformDAGCombine` as an override on
> `Cpu0TargetLowering` (the base class, `Cpu0ISelLowering.h`). The actual implementation
> places it on `Cpu0SETargetLowering` (the SE subclass, `Cpu0SEISelLowering.h`), because
> it is the SE subclass constructor that registers the combine via `setTargetDAGCombine`.
> `Cpu0ISelLowering.h` requires no change.

**What to add** inside `class Cpu0SETargetLowering` in `Cpu0SEISelLowering.h`:

```cpp
SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;
```

---

### 6 · Cpu0SEISelLowering.cpp — Division lowering + DAG combine

**What to add** in `Cpu0SETargetLowering` constructor (after `addRegisterClass`):

```cpp
// Mark MULHS/MULHU as legal — handled by selectMULT in ISelDAGToDAG
setOperationAction(ISD::MULHS, MVT::i32, Legal);
setOperationAction(ISD::MULHU, MVT::i32, Legal);

// Expand signed/unsigned division and remainder to sdivrem/udivrem
setOperationAction(ISD::SDIV, MVT::i32, Expand);
setOperationAction(ISD::SREM, MVT::i32, Expand);
setOperationAction(ISD::UDIV, MVT::i32, Expand);
setOperationAction(ISD::UREM, MVT::i32, Expand);

// Our DAG combine will replace sdivrem/udivrem with Cpu0ISD::DivRem
// LLVM 20: setTargetDAGCombine takes an ArrayRef initializer list
setTargetDAGCombine({ISD::SDIVREM, ISD::UDIVREM});
```

**New static helper** (above `PerformDAGCombine`):

```cpp
static SDValue performDivRemCombine(SDNode *N, SelectionDAG &DAG,
                                    TargetLowering::DAGCombinerInfo &DCI,
                                    const Cpu0Subtarget &Subtarget) {
  if (DCI.isBeforeLegalizeOps()) return SDValue();

  EVT Ty = N->getValueType(0);
  unsigned LO = Cpu0::LO, HI = Cpu0::HI;
  unsigned Opc = (N->getOpcode() == ISD::SDIVREM)
                     ? Cpu0ISD::DivRem
                     : Cpu0ISD::DivRemU;
  SDLoc DL(N);

  SDValue DivRem = DAG.getNode(Opc, DL, MVT::Glue,
                               N->getOperand(0), N->getOperand(1));
  SDValue InChain  = DAG.getEntryNode();
  SDValue InGlue   = DivRem;

  // insert MFLO for quotient (value 0)
  if (N->hasAnyUseOfValue(0)) {
    SDValue CopyFromLo = DAG.getCopyFromReg(InChain, DL, LO, Ty, InGlue);
    DAG.ReplaceAllUsesOfValueWith(SDValue(N, 0), CopyFromLo);
    InChain = CopyFromLo.getValue(1);
    InGlue  = CopyFromLo.getValue(2);
  }
  // insert MFHI for remainder (value 1)
  if (N->hasAnyUseOfValue(1)) {
    SDValue CopyFromHi = DAG.getCopyFromReg(InChain, DL, HI, Ty, InGlue);
    DAG.ReplaceAllUsesOfValueWith(SDValue(N, 1), CopyFromHi);
  }
  return SDValue();
}
```

**New method** in `Cpu0TargetLowering`:

```cpp
SDValue Cpu0SETargetLowering::PerformDAGCombine(SDNode *N,
                                               DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  switch (N->getOpcode()) {
  default: break;
  case ISD::SDIVREM:
  case ISD::UDIVREM:
    return performDivRemCombine(N, DAG, DCI, Subtarget);
  }
  return SDValue();
}
```

---

### 7 · Cpu0SEInstrInfo.h — copyPhysReg declaration

**Why:** When the register allocator needs to copy between a GPR and HI or LO (e.g. to
free up a callee-save slot), it calls `copyPhysReg`. Without this override the default
implementation asserts.

> **Implementation note (LLVM 20):** The tutorial uses `MCRegister` for DestReg/SrcReg
> and a 6-parameter signature. LLVM 20 uses `Register` (which wraps `MCRegister`) and
> adds `RenamableDest` and `RenamableSrc` boolean parameters. The signature below is what
> LLVM 20 requires.

**What to add** in `class Cpu0SEInstrInfo`:

```cpp
void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                 const DebugLoc &DL, Register DestReg, Register SrcReg,
                 bool KillSrc, bool RenamableDest = false,
                 bool RenamableSrc = false) const override;
```

---

### 8 · Cpu0SEInstrInfo.cpp — copyPhysReg implementation

> **Implementation note:** The actual implementation checks `GPROutRegClass` (not
> `CPURegsRegClass`) for the destination. GPR→GPR copies use `ADDu dst, ZERO, src`;
> the MIB building is split by opcode rather than using optional addReg calls.

```cpp
void Cpu0SEInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator I,
                                   const DebugLoc &DL,
                                   Register DestReg, Register SrcReg,
                                   bool KillSrc, bool RenamableDest,
                                   bool RenamableSrc) const {
  unsigned Opc = 0;

  if (Cpu0::GPROutRegClass.contains(DestReg)) {
    if (Cpu0::GPROutRegClass.contains(SrcReg))
      Opc = Cpu0::ADDu;   // GPR -> GPR: ADDu dst, ZERO, src
    else if (SrcReg == Cpu0::HI)
      Opc = Cpu0::MFHI;
    else if (SrcReg == Cpu0::LO)
      Opc = Cpu0::MFLO;
  } else if (DestReg == Cpu0::HI) {
    if (Cpu0::GPROutRegClass.contains(SrcReg))
      Opc = Cpu0::MTHI;
  } else if (DestReg == Cpu0::LO) {
    if (Cpu0::GPROutRegClass.contains(SrcReg))
      Opc = Cpu0::MTLO;
  }

  assert(Opc && "Cannot copy registers");

  MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Opc));
  if (Opc == Cpu0::ADDu) {
    MIB.addReg(DestReg, RegState::Define)
       .addReg(Cpu0::ZERO)
       .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Opc == Cpu0::MFHI || Opc == Cpu0::MFLO) {
    MIB.addReg(DestReg, RegState::Define);
  } else {
    // MTHI / MTLO
    MIB.addReg(SrcReg, getKillRegState(KillSrc));
  }
}
```

---

### 9 · Cpu0ISelDAGToDAG.h — selectMULT declaration

**Why:** LLVM lowers `mulhs`/`mulhu` (multiply high signed/unsigned) to `ISD::MULHS` /
`ISD::MULHU`. TableGen cannot pattern-match these directly because the multiply result
lands in HI/LO and then needs an `MFHI` to get the high word. `selectMULT` does this
manually in C++.

> **Implementation note:** The tutorial returns `std::pair<SDNode*, SDNode*>` with
> parameter order `(HasLo, HasHi)`. The actual implementation uses `void` return and
> calls `ReplaceNode` directly inside `selectMULT`, with parameter order `(HasHi, HasLo)`.

**What to add** in `class Cpu0DAGToDAGISel`:

```cpp
void selectMULT(SDNode *N, unsigned Opc, const SDLoc &DL, EVT Ty,
                bool HasHi, bool HasLo);
```

---

### 10 · Cpu0ISelDAGToDAG.cpp — selectMULT implementation and Select hook

**selectMULT** (void; calls `ReplaceNode` internally):

```cpp
void Cpu0DAGToDAGISel::selectMULT(SDNode *N, unsigned Opc, const SDLoc &DL,
                                    EVT Ty, bool HasHi, bool HasLo) {
  SDNode *Mul = CurDAG->getMachineNode(Opc, DL, MVT::Glue,
                                        N->getOperand(0), N->getOperand(1));
  SDValue InGlue = SDValue(Mul, 0);

  if (HasHi) {
    SDNode *Hi = CurDAG->getMachineNode(Cpu0::MFHI, DL, Ty, MVT::Glue, InGlue);
    InGlue = SDValue(Hi, 1);
    ReplaceNode(N, Hi);
  }
  if (HasLo) {
    SDNode *Lo = CurDAG->getMachineNode(Cpu0::MFLO, DL, Ty, MVT::Glue, InGlue);
    ReplaceNode(N, Lo);
  }
}
```

**In `Select()`** — add cases before `SelectCode(Node)`:

```cpp
unsigned Opcode = Node->getOpcode();
SDLoc DL(Node);

switch (Opcode) {
default: break;
case ISD::MULHS:
  selectMULT(Node, Cpu0::MULT,  DL, MVT::i32, /*HasHi=*/true, /*HasLo=*/false);
  return;
case ISD::MULHU:
  selectMULT(Node, Cpu0::MULTu, DL, MVT::i32, /*HasHi=*/true, /*HasLo=*/false);
  return;
}
```

---

## Acceptance Test

After all changes compile, run:

```bash
# Compile a C file with arithmetic operators
echo 'int test(int a, int b) { return a+b - a*b + (a<<2) + (a>>1); }' \
  | build/bin/clang -target mips-unknown-linux-gnu -emit-llvm -x c - -o /tmp/t.bc
build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/t.bc -o -
```

Expected: assembly contains `addu`, `subu`/`mul`, `shl`, `sra`/`shr` instructions and
terminates with `ret $lr`.

```bash
# Test division / modulo with variable divisor (forces div instruction)
echo 'int test(int a, volatile int b) { return a % b; }' \
  | build/bin/clang -target mips-unknown-linux-gnu -emit-llvm -x c - -o /tmp/m.bc
build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/m.bc -o -
```

Expected: assembly contains `div $zero, ...`, `mfhi ...`.

---

## LLVM API Notes (LLVM 20 vs Tutorial LLVM 12)

| Tutorial (LLVM 12) | LLVM 20 Equivalent |
|---|---|
| `copyPhysReg(MBB, I, DL, MCRegister, MCRegister, bool KillSrc)` | `copyPhysReg(MBB, I, DL, Register, Register, bool KillSrc, bool RenamableDest=false, bool RenamableSrc=false)` |
| `std::pair<SDNode*,SDNode*> selectMULT(..., bool HasLo, bool HasHi)` | `void selectMULT(..., bool HasHi, bool HasLo)` — calls `ReplaceNode` internally |
| `PerformDAGCombine` override on base `Cpu0TargetLowering` | Override placed on `Cpu0SETargetLowering` (SE subclass) |
| `setTargetDAGCombine(ISD::SDIVREM); setTargetDAGCombine(ISD::UDIVREM);` | `setTargetDAGCombine({ISD::SDIVREM, ISD::UDIVREM});` (ArrayRef overload) |
| `SDNodeXForm` with `getImm(N, v)` | `CurDAG->getTargetConstant(v, SDLoc(N), MVT::i32)` inside XForm |
| `immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>` | `immZExt5 : PatLeaf<(imm), [{ return isUInt<5>(N->getZExtValue()); }]>` |
| `getAlignment()` on `MachineMemOperand` | `getAlign()` |
