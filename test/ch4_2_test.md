# Chapter 4.2 Logical Instructions — Test Runbook

## Setup

```bash
BIN=~/workspace/compiler/llvm-project/build/bin
INPUT=~/workspace/compiler/llvm_backend_reference/lbdex/input
```

> **Note:** The warnings below appear on every llc run and are harmless — they come from
> the mips-targeted IR attributes being ignored by the cpu0 backend:
> ```
> 'mips32r2' is not a recognized processor for this target (ignoring processor)
> '+fpxx' is not a recognized feature for this target (ignoring feature)
> ...
> ```

## Rebuild after modifying files

```bash
ninja -C ~/workspace/compiler/llvm-project/build LLVMCpu0CodeGen LLVMCpu0AsmPrinter
```

---

## Test 1: Bitwise AND, OR, XOR, NOT, complement — `ch4_2_logic.cpp` (`test_andorxornotcomplement`)

Covers C operators `&`, `|`, `^`, `!`, `~` mapping to Cpu0 instructions:
`and`, `or`, `xor`, `nor` (plus a `cmp`+`andi`+`shr` sequence for `!`).

Both cpu032I and cpu032II produce identical output here — there is no logical-instruction
difference between the two subtargets. Only the setcc operators (Test 2/3) differ.

**Source (`lbdex/input/ch4_2_logic.cpp`, first function):**
```c
int test_andorxornotcomplement()
{
  int a = 5;
  int b = 3;
  int c = 0, d = 0, e = 0, f = 0, g = 0;

  c = (a & b);  // c = 1
  d = (a | b);  // d = 7
  e = (a ^ b);  // e = 6
  b = !a;       // b = 0
  g = ~f;       // 1's complement, ~0=(-1)=0xffffffff

  return (c+d+e+b+g); // 13
}
```

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_2_logic.cpp -emit-llvm -o /tmp/ch4_2_logic.bc
$BIN/llc -march=cpu0 -mcpu=cpu032I -relocation-model=pic -filetype=asm /tmp/ch4_2_logic.bc -o -
```

**Expected key instructions (`_Z24test_andorxornotcomplementv`, cpu032I):**
```asm
_Z24test_andorxornotcomplementv:
    .frame  $fp,32,$lr
# %bb.0:
    addiu   $sp, $sp, -32
    addiu   $2, $zero, 5
    st      $2, 4($fp)          # a = 5
    addiu   $2, $zero, 3
    st      $2, 0($fp)          # b = 3
    addiu   $2, $zero, 0
    st      $2, -4($fp)         # c = 0
    st      $2, -8($fp)         # d = 0
    st      $2, -12($fp)        # e = 0
    st      $2, -16($fp)        # f = 0
    st      $2, -20($fp)        # g = 0
    ld      $3, 4($fp)
    ld      $4, 0($fp)
    and     $3, $3, $4          # c = a & b  → and
    st      $3, -4($fp)
    ld      $3, 4($fp)
    ld      $4, 0($fp)
    or      $3, $3, $4          # d = a | b  → or
    st      $3, -8($fp)
    ld      $3, 4($fp)
    ld      $4, 0($fp)
    xor     $3, $3, $4          # e = a ^ b  → xor
    st      $3, -12($fp)
    ld      $3, 4($fp)          # load a for !a
    cmp     $sw, $3, $zero      # b = !a: seteq(a, 0) via CMP
    andi    $2, $sw, 2          # isolate Z flag (bit 1 of $sw)
    shr     $2, $2, 1           # shift to bit 0 → b = (a == 0) = 0
    st      $2, 0($fp)
    ld      $2, -16($fp)        # load f (= 0)
    nor     $2, $2, $zero       # g = ~f: 1's complement → NOR(f, $zero)
    st      $2, -20($fp)
    ...                         # accumulate c+d+e+b+g via addu
    addiu   $sp, $sp, 32
    ret     $lr
```

**Instruction mapping:**

| C operator | LLVM IR | Selection DAG | Cpu0 asm |
|---|---|---|---|
| `a & b` | `and` | `and` | `and $rd, $ra, $rb` |
| `a \| b` | `or` | `or` | `or $rd, $ra, $rb` |
| `a ^ b` | `xor` | `xor` | `xor $rd, $ra, $rb` |
| `!a` | `icmp ne a, 0` + `xor i1, true` + `zext` | `(seteq a, 0)` | `cmp $sw, $a, $zero` + `andi $sw, 2` + `shr 1` |
| `~f` | `xor f, -1` | `(not f)` | `nor $rd, $f, $zero` |

> **Why `!a` uses `cmp` not a dedicated instruction:**
> In C, `!x` is `(x == 0)` semantically. LLVM lowers it to `icmp ne x, 0` + logical-not,
> which the DAG simplifies to `seteq(x, 0)`. The setcc pattern match then fires the CMP
> path (cpu032I) or the XOR+SLTiu path (cpu032II).
>
> **Why `~f` uses `nor` not `xori`:**
> The TableGen pattern `(not CPURegs:$in)` matches `xor $in, -1` and lowers it to
> `NOR($in, ZERO)`, exploiting that `NOR(x, 0) = NOT(x)` bitwise.

---

## Test 2: Set-condition instructions with cpu032I (CMP-based) — `test_setxx`

cpu032I uses the `cmp` instruction which writes a status word (`$sw`) encoding four flags
in its low 3 bits (N=bit0, Z=bit1, C=bit2).  Subsequent `andi`/`shr`/`xori` instructions
extract the relevant flag.

**Source (`lbdex/input/ch4_2_logic.cpp`, second function):**
```c
int test_setxx()
{
  int a = 5;
  int b = 3;
  int c, d, e, f, g, h;

  c = (a == b); // seq, c = 0
  d = (a != b); // sne, d = 1
  e = (a < b);  // slt, e = 0
  f = (a <= b); // sle, f = 0
  g = (a > b);  // sgt, g = 1
  h = (a >= b); // sge, h = 1

  return (c+d+e+f+g+h); // 3
}
```

```bash
$BIN/llc -march=cpu0 -mcpu=cpu032I -relocation-model=pic -filetype=asm /tmp/ch4_2_logic.bc -o -
```

**Expected key instructions (`_Z10test_setxxv`, cpu032I):**
```asm
_Z10test_setxxv:
    ...
    # c = (a == b):  seteq → Z flag (bit 1 of $sw)
    cmp     $sw, $3, $2         # CMP(a, b)
    andi    $2, $sw, 2          # isolate bit 1 (Z flag)
    shr     $2, $2, 1           # shift to bit 0: c = 1 if a==b
    st      $2, ...

    # d = (a != b):  setne → Z flag inverted
    cmp     $sw, $3, $2         # CMP(a, b)
    andi    $2, $sw, 2          # isolate bit 1 (Z flag)
    shr     $2, $2, 1           # shift to bit 0
    xori    $2, $2, 1           # invert: d = 1 if a!=b
    st      $2, ...

    # e = (a < b):  setlt → N flag (bit 0 of $sw)
    cmp     $sw, $3, $2         # CMP(a, b)
    andi    $2, $sw, 1          # isolate bit 0 (N flag): e = 1 if a<b
    st      $2, ...

    # f = (a <= b):  setle → NOT(setlt(b, a)) → swap operands
    cmp     $sw, $2, $3         # CMP(b, a)  ← operands swapped
    andi    $2, $sw, 1          # N flag: 1 if b<a (i.e., a>b)
    xori    $2, $2, 1           # invert: f = 1 if a<=b
    st      $2, ...

    # g = (a > b):  setgt → setlt(b, a) → swap operands
    cmp     $sw, $2, $3         # CMP(b, a)  ← operands swapped
    andi    $2, $sw, 1          # N flag: g = 1 if b<a (i.e., a>b)
    st      $2, ...

    # h = (a >= b):  setge → NOT(setlt(a, b))
    cmp     $sw, $3, $2         # CMP(a, b)
    andi    $2, $sw, 1          # N flag: 1 if a<b
    xori    $2, $2, 1           # invert: h = 1 if a>=b
    st      $2, ...
    ...
    ret     $lr
```

**CMP status word bit layout:**

| Bit | Flag | Meaning |
|---|---|---|
| 0 | N (Negative) | Set if signed result < 0 (i.e., `lhs < rhs` signed) |
| 1 | Z (Zero) | Set if `lhs == rhs` |
| 2 | C (Carry) | Set by unsigned comparison (`cmpu`) |

**cpu032I setcc pattern summary:**

| C op | Condition | CMP operand order | Extraction sequence |
|---|---|---|---|
| `==` | seteq | `cmp a, b` | `andi $sw, 2; shr 1` |
| `!=` | setne | `cmp a, b` | `andi $sw, 2; shr 1; xori 1` |
| `<` | setlt | `cmp a, b` | `andi $sw, 1` |
| `<=` | setle | `cmp b, a` | `andi $sw, 1; xori 1` |
| `>` | setgt | `cmp b, a` | `andi $sw, 1` |
| `>=` | setge | `cmp a, b` | `andi $sw, 1; xori 1` |

> **Why `<=` and `>` swap operands:**
> `a <= b` is equivalent to `NOT(b < a)`.  Rather than introducing a dedicated
> `setle` instruction, the backend expresses it as `NOT(setlt(b, a))` by reversing
> the CMP operand order and XOR-ing the N flag with 1.  Similarly `a > b` = `setlt(b, a)`.

---

## Test 3: Set-condition instructions with cpu032II (SLT-based) — `test_setxx`

cpu032II replaces the CMP-based setcc sequences with dedicated `slt`/`sltu`/`slti`/`sltiu`
instructions.  This requires fewer instructions per comparison and avoids the `$sw` status
register entirely.

```bash
$BIN/llc -march=cpu0 -mcpu=cpu032II -relocation-model=pic -filetype=asm /tmp/ch4_2_logic.bc -o -
```

**Expected key instructions (`_Z10test_setxxv`, cpu032II):**
```asm
_Z10test_setxxv:
    ...
    # c = (a == b):  seteq → SLTiu(XOR(a,b), 1)
    xor     $2, $3, $2          # XOR(a, b): 0 iff a==b
    sltiu   $2, $2, 1           # 1 if XOR < 1 (i.e., XOR == 0)
    st      $2, ...

    # d = (a != b):  setne → SLTu(ZERO, XOR(a,b))
    xor     $2, $3, $2          # XOR(a, b): 0 iff a==b
    sltu    $2, $zero, $2       # 1 if 0 < XOR (i.e., XOR != 0)
    st      $2, ...

    # e = (a < b):  setlt → SLT(a, b)
    slt     $2, $3, $2          # 1 if a < b (signed)
    st      $2, ...

    # f = (a <= b):  setle → XORi(SLT(b, a), 1)
    slt     $2, $2, $3          # SLT(b, a): 1 if b < a (i.e., a > b)
    xori    $2, $2, 1           # invert: f = 1 if a<=b
    st      $2, ...

    # g = (a > b):  setgt → SLT(b, a)
    slt     $2, $2, $3          # 1 if b < a (i.e., a > b)
    st      $2, ...

    # h = (a >= b):  setge → XORi(SLT(a, b), 1)
    slt     $2, $3, $2          # SLT(a, b): 1 if a < b
    xori    $2, $2, 1           # invert: h = 1 if a>=b
    st      $2, ...
    ...
    ret     $lr
```

**cpu032II setcc pattern summary:**

| C op | Condition | SLT expansion | Instruction sequence |
|---|---|---|---|
| `==` | seteq | `SLTiu(XOR(a,b), 1)` | `xor $t, a, b; sltiu $rd, $t, 1` |
| `!=` | setne | `SLTu(0, XOR(a,b))` | `xor $t, a, b; sltu $rd, $zero, $t` |
| `<` | setlt | `SLT(a, b)` | `slt $rd, a, b` |
| `<=` | setle | `XORi(SLT(b, a), 1)` | `slt $t, b, a; xori $rd, $t, 1` |
| `>` | setgt | `SLT(b, a)` | `slt $rd, b, a` |
| `>=` | setge | `XORi(SLT(a, b), 1)` | `slt $t, a, b; xori $rd, $t, 1` |

> **Why cpu032II is more efficient:**
> Each comparison takes 1–2 instructions versus 3–4 for cpu032I.
> The `seteq` trick `SLTiu(XOR(a,b), 1)` works because `XOR(a,b) == 0` iff `a == b`,
> and `SLTiu(0, 1)` is 1 (true) while any non-zero value < 1 unsigned is false.

---

## Test 4: SLT immediate optimization — `ch4_2_slt_explain.cpp`

Tests `a < imm` with a compile-time constant RHS.  cpu032II uses `slti` (immediate form)
directly, while cpu032I must materialize the constant in a register before `cmp`.

**Source (`lbdex/input/ch4_2_slt_explain.cpp`):**
```c
int test_OptSlt()
{
  int a = 3, b = 1;
  int d = 0, e = 0, f = 0;

  d = (a < 1);   // slti  $d, $a, 1  (cpu032II)
  e = (b < 2);   // slti  $e, $b, 2  (cpu032II)
  f = d + e;

  return (f);
}
```

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_2_slt_explain.cpp -emit-llvm -o /tmp/ch4_2_slt_explain.bc

# cpu032I: constant must be materialized in a register before CMP
$BIN/llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm /tmp/ch4_2_slt_explain.bc -o -

# cpu032II: slti immediate instruction used directly
$BIN/llc -march=cpu0 -mcpu=cpu032II -relocation-model=static -filetype=asm /tmp/ch4_2_slt_explain.bc -o -
```

**Expected key instructions (cpu032I):**
```asm
_Z11test_OptSltv:
    ...
    addiu   $2, $zero, 3
    st      $2, 0($fp)           # a = 3
    addiu   $2, $zero, 1
    st      $2, -4($fp)          # b = 1
    ld      $2, 0($fp)           # load a
    addiu   $3, $zero, 1         # materialize constant 1
    cmp     $sw, $2, $3          # CMP(a, 1)
    andi    $2, $sw, 1           # N flag: d = 1 if a < 1
    st      $2, -8($fp)
    ld      $2, -4($fp)          # load b
    addiu   $3, $zero, 2         # materialize constant 2
    cmp     $sw, $2, $3          # CMP(b, 2)
    andi    $2, $sw, 1           # N flag: e = 1 if b < 2
    st      $2, -12($fp)
    ...
    ret     $lr
```

**Expected key instructions (cpu032II):**
```asm
_Z11test_OptSltv:
    ...
    ld      $2, 0($fp)           # load a
    slti    $2, $2, 1            # d = 1 if a < 1  (immediate, no register for const)
    st      $2, -8($fp)
    ld      $2, -4($fp)          # load b
    slti    $2, $2, 2            # e = 1 if b < 2  (immediate)
    st      $2, -12($fp)
    ...
    ret     $lr
```

> **Key difference:** cpu032II's `slti` encodes the immediate directly in the instruction
> word, saving a register and an `addiu` per comparison.  This is the motivation for
> having `SetCC_I` (immediate form) alongside `SetCC_R` (register form) in `Cpu0InstrInfo.td`.

---

## Instruction mapping — full operator table (Table 4.5, tutorial p.252)

| C | LLVM IR (.bc) | Optimized legalized selection DAG | cpu032I | cpu032II |
|---|---|---|---|---|
| `&`, `&&` | `and` | `and` | `and $rd, $ra, $rb` | `and $rd, $ra, $rb` |
| `\|`, `\|\|` | `or` | `or` | `or $rd, $ra, $rb` | `or $rd, $ra, $rb` |
| `^` | `xor` | `xor` | `xor $rd, $ra, $rb` | `xor $rd, $ra, $rb` |
| `!` | `icmp ne a, 0` + `xor true` + `zext` | `(seteq a, 0)` | `cmp; andi 2; shr 1` | `sltiu $rd, $a, 1` |
| `==` | `icmp eq` + `zext` | `(seteq a, b)` | `cmp; andi 2; shr 1` | `xor; sltiu 1` |
| `!=` | `icmp ne` + `zext` | `(setne a, b)` | `cmp; andi 2; shr 1; xori 1` | `xor; sltu $zero, t` |
| `<` | `icmp slt` + `zext` | `(setlt a, b)` | `cmp; andi 1` | `slt $rd, a, b` |
| `<=` | `icmp sle` + `zext` | `(setle a, b)` | `cmp(b,a); andi 1; xori 1` | `slt(b,a); xori 1` |
| `>` | `icmp sgt` + `zext` | `(setgt a, b)` | `cmp(b,a); andi 1` | `slt $rd, b, a` |
| `>=` | `icmp sge` + `zext` | `(setge a, b)` | `cmp(a,b); andi 1; xori 1` | `slt(a,b); xori 1` |

> **Note on `lbdex/input/ch4_2_logic.cpp`:** This file is from the external tutorial
> distribution and lives at `$INPUT/ch4_2_logic.cpp` (i.e.,
> `~/workspace/compiler/llvm_backend_reference/lbdex/input/ch4_2_logic.cpp`).
> It is **not** checked into the llvm-project repo.
