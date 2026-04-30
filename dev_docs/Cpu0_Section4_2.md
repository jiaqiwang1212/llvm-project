# Cpu0 Backend — Chapter 4.2: Logical Instructions

## Overview

Chapter 4.2 extends the Cpu0 backend with logical operators and comparison-based
set-condition-code (setcc) instructions. The C operators covered are:

```
&  |  ^  ~  !  ==  !=  <  <=  >  >=
```

These are gated behind the `Ch4_2` chapter predicate in `Cpu0InstrInfo.td`.
The implementation also adds `SIGN_EXTEND_INREG` expansion in
`Cpu0SEISelLowering.cpp` since Cpu0 has no native sign-extension instruction.

---

## 1. New Instruction Classes

### 1a. `CmpInstr` — Compare (cpu032I variant)

```tablegen
class CmpInstr<bits<8> op, string instr_asm,
               InstrItinClass itin, RegisterClass RC, RegisterClass RD,
               bit isComm = 0> :
  FA<op, (outs RD:$ra), (ins RC:$rb, RC:$rc), ...>
```

- Writes the comparison result to the **SW** (status word) register via the `SR`
  register class, not to a GPR.
- Has no DAG pattern in the class body; matching is done via `setcc` Pat rules.
- Two instruction defs under `[Ch4_2, HasCmp]`:
  | Mnemonic | Opcode | Description |
  |----------|--------|-------------|
  | `CMP`    | 0x2A   | Signed compare |
  | `CMPu`   | 0x2B   | Unsigned compare |

### 1b. `LogicNOR` — NOR with embedded pattern

```tablegen
class LogicNOR<bits<8> op, string instr_asm, RegisterClass RC> :
  FA<op, ..., [(set RC:$ra, (not (or RC:$rb, RC:$rc)))], IIAlu>
```

- The `not (or ...)` pattern is embedded in the class body, so TableGen
  automatically selects NOR for that DAG shape.
- `NOR` def (opcode 0x1b) is placed under `[Ch4_2]`.

### 1c. `SetCC_R` / `SetCC_I` — Set-Less-Than (cpu032II variant)

Both classes carry `Requires<[HasSlt]>` so they are only selected when the
cpu032II subtarget is active.

---

## 2. New Instruction Definitions

All new defs are gated to `Ch4_2` (or `Ch4_2 + HasSlt/HasCmp`).

### 2a. Immediate Logical Instructions

| Mnemonic | Opcode | IR Operation | Immediate type |
|----------|--------|--------------|----------------|
| `ANDi`   | 0x0c   | `and`        | `immZExt16` (uimm16) |
| `XORi`   | 0x0e   | `xor`        | `immZExt16` (uimm16) |

Both use the existing `ArithLogicI` template.  `ORi` (0x0d) was already present
under `Ch3_5` and is unchanged.

### 2b. Register-Register Logical Instructions

| Mnemonic | Opcode | IR Operation | Gate |
|----------|--------|--------------|------|
| `AND`    | 0x18   | `and`        | Ch4_2 |
| `OR`     | 0x19   | `or`         | Ch4_2 |
| `XOR`    | 0x1a   | `xor`        | Ch4_2 |
| `NOR`    | 0x1b   | `not (or …)` | Ch4_2 |

These were previously gated to `Ch4_1` — moving them to `Ch4_2` matches the
canonical tutorial chapter structure.

### 2c. Set-Less-Than Instructions (cpu032II, `HasSlt`)

| Mnemonic | Opcode | Condition | Format |
|----------|--------|-----------|--------|
| `SLTi`   | **0x26** | `setlt` (signed)    | FL (reg-imm) |
| `SLTiu`  | **0x27** | `setult` (unsigned) | FL (reg-imm) |
| `SLT`    | 0x28   | `setlt` (signed)    | FA (reg-reg) |
| `SLTu`   | 0x29   | `setult` (unsigned) | FA (reg-reg) |

> **Opcode corrections from Ch4_1 prototype:**
> - `SLTi` was incorrectly 0x2a → corrected to **0x26**
> - `SLTiu` was incorrectly 0x2b → corrected to **0x27**

### 2d. Compare Instructions (cpu032I, `HasCmp`)

| Mnemonic | Opcode | Description |
|----------|--------|-------------|
| `CMP`    | **0x2A** | Signed compare; writes to SW/SR |
| `CMPu`   | **0x2B** | Unsigned compare; writes to SW/SR |

> **Opcode correction:** `CMP` was incorrectly 0x2c → corrected to **0x2A**.
> `CMPu` is new (was entirely absent from Ch4_1).

---

## 3. `not` Pattern

```tablegen
let Predicates = [Ch4_2] in
def : Pat<(not CPURegs:$in), (NOR CPURegs:$in, ZERO)>;
```

This handles the C unary `~` operator (1's complement). It was previously under
`Ch4_1`; moved to `Ch4_2` to match the chapter structure.

---

## 4. setcc Pattern Multiclasses

All multiclasses are defined inside `let Predicates = [Ch4_2] in { ... }`.

### 4a. CMP variant (cpu032I, `HasCmp`)

SW bit layout after `CMP a, b`:
- Bit 0: negative flag (a < b signed)
- Bit 1: zero flag (a == b)

| Multiclass | Patterns |
|------------|---------|
| `SeteqPatsCmp` | `seteq`, `setne` |
| `SetltPatsCmp` | `setlt`, `setult` |
| `SetlePatsCmp` | `setle`, `setule` |
| `SetgtPatsCmp` | `setgt`, `setugt` |
| `SetgePatsCmp` | `setge`, `setuge` |

Example — `a == b`:
```tablegen
def : Pat<(seteq RC:$lhs, RC:$rhs),
          (SHR (ANDi (CMP RC:$lhs, RC:$rhs), 2), 1)>;
```

### 4b. SLT variant (cpu032II, `HasSlt`)

| Multiclass | Patterns |
|------------|---------|
| `SeteqPatsSlt` | `seteq`, `setne` |
| `SetlePatsSlt` | `setle`, `setule` |
| `SetgtPatsSlt` | `setgt`, `setugt` |
| `SetgePatsSlt` | `setge`, `setuge` |
| `SetgeImmPatsSlt` | `setge imm`, `setuge imm` |

`SetgeImmPatsSlt` signature and instantiation:
```tablegen
multiclass SetgeImmPatsSlt<RegisterClass RC,
                           Instruction SLTiOp, Instruction SLTiuOp> {
  def : Pat<(setge RC:$lhs, immSExt16:$rhs),
            (XORi (SLTiOp RC:$lhs, immSExt16:$rhs), 1)>;
  def : Pat<(setuge RC:$lhs, immSExt16:$rhs),
            (XORi (SLTiuOp RC:$lhs, immSExt16:$rhs), 1)>;
}
// instantiated as:
defm : SetgeImmPatsSlt<CPURegs, SLTi, SLTiu>;
```

Example — `a == b` via SLT:
```tablegen
def : Pat<(seteq RC:$lhs, RC:$rhs),
          (SLTiu (XOR RC:$lhs, RC:$rhs), 1)>;
```

---

## 5. `Cpu0SEISelLowering.cpp` — SIGN_EXTEND_INREG

Added to `Cpu0SETargetLowering::Cpu0SETargetLowering` constructor:

```cpp
// Cpu0 has no sext_inreg; expand to shl/sra pairs.
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1,    Expand);
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8,    Expand);
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16,   Expand);
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32,   Expand);
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::Other, Expand);
```

Without these, LLVM would attempt to emit `sext_inreg` as a single instruction,
which Cpu0 cannot do. With `Expand`, LLVM decomposes it into an `SHL` followed
by an `SRA` pair.

---

## 6. Files Modified

| File | Change |
|------|--------|
| `llvm/lib/Target/Cpu0/Cpu0InstrInfo.td` | New classes (CmpInstr, LogicNOR); AND/OR/XOR/NOR moved to Ch4_2; ANDi/XORi/CMPu added; SLT/CMP opcodes corrected; setcc multiclasses added |
| `llvm/lib/Target/Cpu0/Cpu0SEISelLowering.cpp` | SIGN_EXTEND_INREG expansion added |
| `llvm/lib/Target/Cpu0/Cpu0Subtarget.h` | `hasChapter4_2()` accessor added |

---

## 7. Test Verification

The reference test file is `lbdex/input/ch4_2_logic.cpp` from the tutorial distribution
(not checked in to this repo). Use the tutorial's external `lbdex/` tree or a local copy.

```bash
# Compile the Chapter 4.2 test (requires clang targeting mips)
clang -target mips-unknown-linux-gnu -c \
  lbdex/input/ch4_2_logic.cpp -emit-llvm -o ch4_2_logic.bc

# Generate Cpu0 assembly (cpu032I, uses CMP/CMPu)
llc -march=cpu0 -mcpu=cpu032I -relocation-model=pic \
    -filetype=asm ch4_2_logic.bc -o -

# Generate Cpu0 assembly (cpu032II, uses SLT family)
llc -march=cpu0 -mcpu=cpu032II -relocation-model=pic \
    -filetype=asm ch4_2_logic.bc -o -
```

Expected key instructions in cpu032I output (bitwise ops + seteq example):
```asm
and   $3, $4, $3        # a & b
or    $3, $4, $3        # a | b
xor   $3, $4, $3        # a ^ b
cmp   $sw, $3, $2       # compare for setcc
andi  $2, $sw, 2        # mask zero flag (bit 1) for ==
shr   $2, $2, 1         # shift to boolean (seteq)
andi  $2, $sw, 1        # mask negative flag (bit 0) for <
```

Expected key instructions in cpu032II output (bitwise ops + seteq/setge examples):
```asm
and   $3, $4, $3        # a & b
or    $3, $4, $3        # a | b
xor   $3, $4, $3        # a ^ b
xor   $2, $3, $4        # a XOR b (for seteq)
sltiu $3, $2, 1         # seteq: (a XOR b) < 1u
sltu  $3, $0, $2        # setne: 0 < (a XOR b)
slt   $3, $2, $3        # setgt: b < a
xori  $3, $3, 1         # setle/setge: invert less-than
```
