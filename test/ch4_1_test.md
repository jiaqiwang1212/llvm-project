# Chapter 4.1 Arithmetic — Test Runbook

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

## Test 1: Basic arithmetic — `ch4_1_math.cpp`

Covers C operators `+`, `-`, `*`, `<<`, `>>` mapping to Cpu0 instructions:
`addu`, `subu`, `mul`, `shl`, `sra`, `shr`, `shlv`, `srav`, `shrv`.

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_math.cpp -emit-llvm -o /tmp/ch4_1_math.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_math.bc -o -
```

**Expected output:**
```asm
_Z9test_mathv:
	.frame	$fp,64,$lr
# %bb.0:
	addiu	$sp, $sp, -64
	addiu	$2, $zero, 5
	st	$2, 0($fp)
	addiu	$2, $zero, 2
	st	$2, -4($fp)
	addiu	$2, $zero, -5
	st	$2, -8($fp)
	ld	$2, 0($fp)
	ld	$3, -4($fp)
	addu	$2, $2, $3           # c = a + b
	st	$2, -12($fp)
	ld	$2, 0($fp)
	ld	$3, -4($fp)
	subu	$2, $2, $3           # d = a - b
	st	$2, -16($fp)
	ld	$2, 0($fp)
	ld	$3, -4($fp)
	mul	$2, $2, $3            # e = a * b
	st	$2, -20($fp)
	ld	$2, 0($fp)
	shl	$2, $2, 2             # f = a << 2 (constant shift)
	st	$2, -24($fp)
	ld	$2, -8($fp)
	shl	$2, $2, 1             # f1 = a1 << 1
	st	$2, -44($fp)
	ld	$2, 0($fp)
	sra	$2, $2, 2             # g = a >> 2 (arithmetic, constant)
	st	$2, -28($fp)
	ld	$2, -8($fp)
	shr	$2, $2, 30            # g1 = a1 >> 30 (logical, constant)
	st	$2, -48($fp)
	ld	$2, 0($fp)
	addiu	$3, $zero, 1
	shlv	$2, $3, $2            # h = 1 << a (variable shift)
	st	$2, -32($fp)
	ld	$2, -4($fp)
	shlv	$2, $3, $2            # h1 = 1 << b
	st	$2, -52($fp)
	ld	$2, 0($fp)
	addiu	$3, $zero, 128
	shrv	$2, $3, $2            # i = 0x80 >> a (variable shift)
	st	$2, -36($fp)
	ld	$2, -4($fp)
	ld	$3, 0($fp)
	srav	$2, $2, $3            # j = b >> a (arithmetic variable shift)
	st	$2, -56($fp)
	addiu	$2, $zero, -6
	st	$2, -40($fp)
	...                           # sum all into return value via addu
	addiu	$sp, $sp, 64
	ret	$lr
```

**Instruction mapping summary:**

| C operator | LLVM IR | Cpu0 asm | Notes |
|---|---|---|---|
| `a + b` | `add nsw` | `addu` | default (no overflow trap) |
| `a - b` | `sub nsw` | `subu` | default (no overflow trap) |
| `a * b` | `mul nsw` | `mul` | low 32 bits only |
| `a << 2` | `shl` (const) | `shl` | immediate shift |
| `a >> 2` signed | `ashr` (const) | `sra` | arithmetic shift right |
| `a >> 30` unsigned | `lshr` (const) | `shr` | logical shift right |
| `1 << a` | `shl` (variable) | `shlv` | variable left shift |
| `0x80 >> a` | `lshr` (variable) | `shrv` | variable logical shift right |
| `b >> a` signed | `ashr` (variable) | `srav` | variable arithmetic shift right |

---

## Test 2: Add/Sub with overflow trapping — `ch4_1_addsuboverflow.cpp`

With `-cpu0-enable-overflow=true`, `add` and `sub` instructions are emitted instead of
`addu`/`subu`. These trigger a CPU overflow exception on signed overflow.

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_addsuboverflow.cpp \
    -emit-llvm -o /tmp/ch4_1_addsuboverflow.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm \
    -cpu0-enable-overflow=true /tmp/ch4_1_addsuboverflow.bc -o -
```

**Expected output:**
```asm
_Z17test_add_overflowv:
	.frame	$fp,16,$lr
# %bb.0:
	addiu	$sp, $sp, -16
	lui	$2, 28672          # 0x70000000 (upper 16 bits = 0x7000)
	st	$2, 0($fp)
	lui	$2, 8192           # 0x20000000 (upper 16 bits = 0x2000)
	st	$2, -4($fp)
	addiu	$2, $zero, 0
	st	$2, -8($fp)
	ld	$3, 0($fp)
	ld	$4, -4($fp)
	add	$3, $3, $4         # overflow-trapping add (not addu)
	st	$3, -8($fp)
	addiu	$sp, $sp, 16
	ret	$lr

_Z17test_sub_overflowv:
	.frame	$fp,16,$lr
# %bb.0:
	addiu	$sp, $sp, -16
	lui	$2, 36864          # -0x70000000 upper bits
	st	$2, 0($fp)
	lui	$2, 8192
	st	$2, -4($fp)
	addiu	$2, $zero, 0
	st	$2, -8($fp)
	ld	$3, 0($fp)
	ld	$4, -4($fp)
	sub	$3, $3, $4         # overflow-trapping sub (not subu)
	st	$3, -8($fp)
	addiu	$sp, $sp, 16
	ret	$lr
```

> **Key point:** Without `-cpu0-enable-overflow=true`, the same source produces
> `addu`/`subu`. Modern code normally uses truncating arithmetic; the overflow
> variant is mainly for debugging.

---

## Test 3: Modulo with constant divisor — `ch4_1_mult.cpp`

Divisor is the compile-time constant `12`. LLVM replaces `srem` with a
multiply-based sequence (cheaper than division).

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_mult.cpp -emit-llvm -o /tmp/ch4_1_mult.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_mult.bc -o -
```

**Expected output:**
```asm
_Z9test_multv:
	.frame	$fp,8,$lr
# %bb.0:
	addiu	$sp, $sp, -8
	addiu	$2, $zero, 11
	st	$2, 0($fp)
	ld	$2, 0($fp)
	addiu	$2, $2, 1          # b + 1 = 12
	lui	$3, 10922
	ori	$3, $3, 43691       # magic constant 0x2AAAAAAB for div-by-12
	mult	$2, $3             # MULHS: high word of signed multiply
	mfhi	$3
	shr	$4, $3, 31
	shr	$3, $3, 1
	addu	$3, $3, $4
	addiu	$4, $zero, 12
	mul	$3, $3, $4
	subu	$2, $2, $3         # remainder = dividend - quotient*12
	st	$2, 0($fp)
	ld	$2, 0($fp)
	addiu	$sp, $sp, 8
	ret	$lr
```

> **Why no `div`?** LLVM's constant-divisor optimization replaces `srem x, 12`
> with `mulhs(x, magic) + adjustments`. The `mult`/`mfhi` sequence extracts the
> high 32 bits of a 64-bit signed multiply.

---

## Test 4: Modulo with non-constant divisor — `ch4_1_mult2.cpp`

Divisor is `int a = 12` (non-volatile). LLVM may apply constant propagation or
fall through to actual division depending on optimization level.

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_mult2.cpp -emit-llvm -o /tmp/ch4_1_mult2.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_mult2.bc -o -
```

**Expected output:**
```asm
_Z9test_multv:
	.frame	$fp,8,$lr
# %bb.0:
	addiu	$sp, $sp, -8
	addiu	$2, $zero, 11
	st	$2, 0($fp)
	addiu	$2, $zero, 12
	st	$2, -4($fp)
	ld	$2, 0($fp)
	addiu	$2, $2, 1
	ld	$3, -4($fp)
	div	$2, $3             # signed division
	mfhi	$2               # remainder lives in HI register
	st	$2, 0($fp)
	ld	$2, 0($fp)
	addiu	$sp, $sp, 8
	ret	$lr
```

> **Note:** Unlike `ch4_1_mult.cpp`, this version stores `a` in a stack slot,
> so LLVM emits actual `div`/`mfhi` rather than the multiply trick.

---

## Test 5: Modulo with volatile divisor — `ch4_1_mod.cpp`

Divisor is `volatile int a = 12`. The `volatile` qualifier prevents any
compile-time optimisation, guaranteeing a real division instruction.

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_mod.cpp -emit-llvm -o /tmp/ch4_1_mod.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_mod.bc -o -
```

**Expected output:**
```asm
_Z8test_modv:
	.frame	$fp,8,$lr
# %bb.0:
	addiu	$sp, $sp, -8
	addiu	$2, $zero, 11
	st	$2, 0($fp)
	addiu	$2, $zero, 12
	st	$2, -4($fp)
	ld	$2, 0($fp)
	addiu	$2, $2, 1
	ld	$3, -4($fp)
	div	$2, $3             # signed division (SDIV → div + mfhi)
	mfhi	$2               # remainder = HI register
	st	$2, 0($fp)
	ld	$2, 0($fp)
	addiu	$sp, $sp, 8
	ret	$lr
```

**DAG translation pipeline for `%` operator:**

| Stage | DAG node |
|---|---|
| LLVM IR (.bc) | `srem` |
| Legalized selection DAG | `sdivrem` |
| Optimized legalized DAG | `Cpu0ISD::DivRem` + `CopyFromReg` |
| Pattern match | `div` (SDIV) + `mfhi` (MFLO) |

---

## Test 6: Division operator — `ch4_1_div.cpp`

Tests both signed (`/`) and unsigned (`/`) division.
`sdiv` → `div` + `mflo` (quotient in LO);
`udiv` → `divu` + `mflo`.

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_div.cpp -emit-llvm -o /tmp/ch4_1_div.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_div.bc -o -
```

**Expected output:**
```asm
_Z8test_divv:
	.frame	$fp,24,$lr
# %bb.0:
	addiu	$sp, $sp, -24
	addiu	$2, $zero, -5
	st	$2, 0($fp)
	addiu	$3, $zero, 2
	st	$3, -4($fp)
	lui	$3, 256             # 0x1000000
	st	$3, -8($fp)
	addiu	$3, $zero, 0
	st	$3, -12($fp)
	st	$2, -16($fp)
	st	$3, -20($fp)
	ld	$2, 0($fp)
	ld	$3, -4($fp)
	div	$2, $3             # signed: d = a / b  (-5 / 2 = -2)
	mflo	$2               # quotient in LO
	st	$2, -12($fp)
	ld	$2, -16($fp)
	ld	$3, -8($fp)
	divu	$2, $3            # unsigned: d1 = a1 / c
	mflo	$2               # quotient in LO
	st	$2, -20($fp)
	ld	$2, -12($fp)
	ld	$3, -20($fp)
	addu	$2, $2, $3
	addiu	$sp, $sp, 24
	ret	$lr
```

**Instruction mapping:**

| C operator | LLVM IR | Cpu0 asm | Result register |
|---|---|---|---|
| `a / b` (signed) | `sdiv` | `div` + `mflo` | LO (quotient) |
| `a % b` (signed) | `srem` | `div` + `mfhi` | HI (remainder) |
| `a / b` (unsigned) | `udiv` | `divu` + `mflo` | LO (quotient) |
| `a % b` (unsigned) | `urem` | `divu` + `mfhi` | HI (remainder) |

---

## Test 7: Rotate instructions — `ch4_1_rotate.cpp`

The C idiom `(a << n) | (a >> (32-n))` is recognized by LLVM as a rotate and
lowered to the Cpu0 `rol` instruction.

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_rotate.cpp -emit-llvm -o /tmp/ch4_1_rotate.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_rotate.bc -o -
```

**Expected output:**
```asm
_Z16test_rotate_leftv:
	.frame	$fp,8,$lr
# %bb.0:
	addiu	$sp, $sp, -8
	addiu	$2, $zero, 8
	st	$2, 0($fp)
	ld	$2, 0($fp)
	rol	$2, $2, 30         # rotate left by 30 (constant)
	st	$2, -4($fp)
	ld	$2, -4($fp)
	addiu	$sp, $sp, 8
	ret	$lr
```

> **How LLVM recognizes the rotate pattern:**
> `(a << 30) | (a >> 2)` with `unsigned int a` →
> LLVM IR: `%2 = shl i32 %1, 30` + `%4 = lshr i32 %3, 2` + `%5 = or i32 %2, %4` →
> DAG: `rotl` node → Cpu0 `ROL` instruction.
>
> Variable rotate (`rolv`/`rorv`) requires `#define TEST_ROXV` in the source file.
> To test that path:
> ```bash
> $BIN/clang -target mips-unknown-linux-gnu -DTEST_ROXV \
>     -c $INPUT/ch4_1_rotate.cpp -emit-llvm -o /tmp/ch4_1_rotate_var.bc
> $BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_rotate_var.bc -o -
> ```
> Expected: `rolv`/`rorv` instructions for the volatile variable rotate functions.

---

## Test 8: Subtraction with large constants — `ch4_1_sub.cpp`

Tests `subu` with values that require `lui`+`ori` to synthesize (>16-bit immediates).

```bash
# Compile without stdio.h for cross-compilation compatibility
cat > /tmp/ch4_1_sub_test.cpp << 'EOF'
int test_sub()
{
  int a = 0x7FFFFFFF;
  int b = 0x80000001;
  int d = a - b;
  return d;
}
EOF
$BIN/clang -target mips-unknown-linux-gnu -c /tmp/ch4_1_sub_test.cpp -emit-llvm -o /tmp/ch4_1_sub.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_sub.bc -o -
```

**Expected output:**
```asm
_Z8test_subv:
	.frame	$fp,16,$lr
# %bb.0:
	addiu	$sp, $sp, -16
	lui	$2, 32767
	ori	$2, $2, 65535      # synthesize 0x7FFFFFFF (>16 bits needs lui+ori)
	st	$2, 0($fp)
	lui	$2, 32768
	ori	$2, $2, 1          # synthesize 0x80000001
	st	$2, -4($fp)
	ld	$2, 0($fp)
	ld	$3, -4($fp)
	subu	$2, $2, $3
	st	$2, -8($fp)
	ld	$2, -8($fp)
	addiu	$sp, $sp, 16
	ret	$lr
```

> **32-bit constant synthesis:** When an immediate exceeds 16 bits, Cpu0 uses:
> - `lui $r, HI16(val)` — load upper 16 bits
> - `ori $r, $r, LO16(val)` — OR in lower 16 bits

---

## Complete instruction coverage for Chapter 4.1

| Instruction | Opcode | Source | Test file |
|---|---|---|---|
| `addu` | 0x11 | C `+` (default) | `ch4_1_math.cpp` |
| `subu` | 0x12 | C `-` (default) | `ch4_1_math.cpp`, `ch4_1_sub.cpp` |
| `add` | 0x13 | C `+` (overflow) | `ch4_1_addsuboverflow.cpp` |
| `sub` | 0x14 | C `-` (overflow) | `ch4_1_addsuboverflow.cpp` |
| `mul` | 0x17 | C `*` (low 32-bit) | `ch4_1_math.cpp` |
| `shl` | 0x1e | C `<<` (constant) | `ch4_1_math.cpp` |
| `sra` | 0x20 | C `>>` signed (constant) | `ch4_1_math.cpp` |
| `shr` | 0x1f | C `>>` unsigned (constant) | `ch4_1_math.cpp` |
| `shlv` | 0x22 | C `<<` (variable) | `ch4_1_math.cpp` |
| `srav` | 0x21 | C `>>` signed (variable) | `ch4_1_math.cpp` |
| `shrv` | 0x23 | C `>>` unsigned (variable) | `ch4_1_math.cpp` |
| `rol` | 0x1c | C rotate idiom (constant) | `ch4_1_rotate.cpp` |
| `rolv` | 0x24 | C rotate idiom (variable) | `ch4_1_rotate.cpp` (+`TEST_ROXV`) |
| `rorv` | 0x25 | C rotate idiom (variable) | `ch4_1_rotate.cpp` (+`TEST_ROXV`) |
| `mult` | 0x41 | MULHS (srem constant opt) | `ch4_1_mult.cpp` |
| `mfhi` | 0x46 | read HI register (remainder) | `ch4_1_mod.cpp`, `ch4_1_mult.cpp` |
| `mflo` | 0x47 | read LO register (quotient) | `ch4_1_div.cpp` |
| `div` | 0x43 | C `/` or `%` signed | `ch4_1_div.cpp`, `ch4_1_mod.cpp` |
| `divu` | 0x44 | C `/` or `%` unsigned | `ch4_1_div.cpp` |

---

## Quick workflow

```bash
ninja -C ~/workspace/compiler/llvm-project/build LLVMCpu0CodeGen LLVMCpu0AsmPrinter && \
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_math.cpp -emit-llvm -o /tmp/ch4_1_math.bc && \
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_math.bc -o -
```
