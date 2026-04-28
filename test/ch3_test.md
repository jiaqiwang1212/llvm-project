# Chapter 3 Backend Structure — Test Runbook

## Setup

```bash
BIN=~/workspace/compiler/llvm-project/build/bin
INPUT=~/workspace/compiler/llvm_backend_reference/lbdex/input
CHAPTER_H=~/workspace/compiler/llvm-project/llvm/lib/Target/Cpu0/Cpu0SetChapter.h
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

## Pre-generate ch3.bc (needed by all sections)

Run once and reuse across section tests.

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch3.cpp -emit-llvm -o /tmp/ch3.bc
$BIN/llvm-dis /tmp/ch3.bc -o -
```

**IR output:**
```llvm
define dso_local noundef i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  ret i32 0
}
```

---

## Section 3.1 — TargetMachine structure

Edit `Cpu0SetChapter.h`:
```cpp
#define CH  CH3_1
```

Rebuild, then:

```bash
# Show available CPUs and features
$BIN/llc -march=cpu0 -mcpu=help
```

**Expected output:**
```
Available CPUs for this target:

  cpu032I  - Select the cpu032I processor.
  cpu032II - Select the cpu032II processor.

Available features for this target:

  ch3_1    - Enable Chapter instructions..
  ch3_2    - Enable Chapter instructions..
  ...
```

```bash
# Expected error: "Unable to create reg info"
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch3.bc -o /tmp/ch3.cpu0.s
```

**Expected error:**
```
LLVM ERROR: Unable to create reg info
```

---

## Section 3.2 — Add AsmPrinter

Edit `Cpu0SetChapter.h`:
```cpp
#define CH  CH3_2
```

Rebuild, then:

```bash
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch3.bc -o /tmp/ch3.cpu0.s
```

**Expected error:**
```
error: target does not support generation of this file type!
```

---

## Section 3.3 — Add Cpu0DAGToDAGISel

Edit `Cpu0SetChapter.h`:
```cpp
#define CH  CH3_3
```

Rebuild, then:

```bash
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch3.bc -o /tmp/ch3.cpu0.s
```

**Expected error:**
```
LLVM ERROR: Cannot select: t5: i32 = Cpu0ISD::Ret t3, t4
```

---

## Section 3.4 — Handle return register $lr

Edit `Cpu0SetChapter.h`:
```cpp
#define CH  CH3_4
```

Rebuild, then:

```bash
# Without -O2: hangs (stack slot not handled yet) — press Ctrl+C to abort
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch3.bc -o -
```

**Expected behavior:** process hangs indefinitely (infinite loop in register allocator trying to spill a virtual register with no spill method). Kill with Ctrl+C.

```bash
# With -O2: optimizer eliminates the alloca entirely — works
$BIN/clang -O2 -target mips-unknown-linux-gnu -c $INPUT/ch3.cpp -emit-llvm -o /tmp/ch3_O2.bc
$BIN/llvm-dis /tmp/ch3_O2.bc -o -
```

**-O2 IR output** (alloca eliminated):
```llvm
define dso_local noundef i32 @main() local_unnamed_addr #0 {
  ret i32 0
}
```

```bash
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch3_O2.bc -o -
```

**Expected output** (no stack frame):
```asm
	.section .mdebug.abiO32
	.previous
	.file	"ch3.cpp"
	.text
	.globl	main
	.type	main,@function
	.ent	main                            # @main
main:
	.frame	$sp,0,$lr
	.mask 	0x00000000,0
	.set	noreorder
	.set	nomacro
# %bb.0:
	ret	$lr
	.set	macro
	.set	reorder
	.end	main
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
```

```bash
# Debug: dump DAG before/after every pass
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm \
    -print-before-all -print-after-all /tmp/ch3_O2.bc -o -
```

---

## Section 3.5 — Add Prologue/Epilogue functions

Edit `Cpu0SetChapter.h`:
```cpp
#define CH  CH3_5
```

Rebuild, then:

### Test 1: simple main() with local variable (no -O2)

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch3.cpp -emit-llvm -o /tmp/ch3.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch3.bc -o -
```

**Expected output:**
```asm
	.section .mdebug.abiO32
	.previous
	.file	"ch3.cpp"
	.text
	.globl	main
	.type	main,@function
	.ent	main                            # @main
main:
	.frame	$fp,8,$lr
	.mask 	0x00000000,0
	.set	noreorder
	.set	nomacro
# %bb.0:
	addiu	$sp, $sp, -8
	addiu	$2, $zero, 0
	st	$2, 0($fp)
	addiu	$sp, $sp, 8
	ret	$lr
	.set	macro
	.set	reorder
	.end	main
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
```

### Test 2: large stack frame

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch3_largeframe.cpp \
    -emit-llvm -o /tmp/ch3_largeframe.bc
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch3_largeframe.bc -o -
```

**Expected output** (our implementation uses a chunked ADDiu loop, frame size = 1,879,015,424 bytes):

```asm
_Z16test_largegframev:
	.frame	$fp,1879015424,$lr
	.mask 	0x00000000,0
	.set	noreorder
	.set	nomacro
# %bb.0:
	addiu	$sp, $sp, -32767   # prologue: repeated 57,332 times
	addiu	$sp, $sp, -32767
	... (57,330 more identical lines)
	addiu	$sp, $sp, -32767   # last full chunk
	addiu	$sp, $sp, -24576   # remainder chunk (1879015424 mod 32767)
	addiu	$2, $zero, 0
	addiu	$sp, $sp, 32767    # epilogue: same number of chunks
	addiu	$sp, $sp, 32767
	... (57,330 more identical lines)
	addiu	$sp, $sp, 32767
	addiu	$sp, $sp, 24576    # remainder restored
	ret	$lr
```

> **Implementation note:** The tutorial reference (Chapter3_5) uses `LUi`/`ORi`/`ADDu` to
> load the full 32-bit stack size into a register in 2–3 instructions. Our current
> implementation uses a chunked `ADDiu` loop (one instruction per 32767-byte chunk),
> which is correct but verbose for very large frames. The LUi-based approach is added
> in a later chapter refinement.

---

## Quick workflow (after each file edit)

```bash
ninja -C ~/workspace/compiler/llvm-project/build LLVMCpu0CodeGen LLVMCpu0AsmPrinter && \
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch3.bc -o -
```
