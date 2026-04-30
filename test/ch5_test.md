# Chapter 5 Generating Object Files - Test Runbook

This runbook is based on every test/run command shown in Chapter 5 of
`TutorialLLVMBackendCpu0.pdf`. The PDF commands assume the tutorial build layout;
the second section gives the equivalent commands for this workspace.

## Setup

```bash
BIN=~/workspace/compiler/llvm-project/build/bin
INPUT=~/workspace/compiler/llvm_backend_reference/lbdex/input
```

Chapter 5 assumes `ch4_1_math.bc` already exists. In this workspace, create it
with:

```bash
$BIN/clang -target mips-unknown-linux-gnu -c $INPUT/ch4_1_math.cpp -emit-llvm -o /tmp/ch4_1_math.bc
```

Rebuild after changing Chapter 5 MC/ELF code:

```bash
cmake --build ~/workspace/compiler/llvm-project/build --target LLVMCpu0Desc
cmake --build ~/workspace/compiler/llvm-project/build --target llc
```

## PDF Command Inventory

These are the Chapter 5 commands as they appear in the PDF, in order.

### Pre-Chapter-5 Failure

```bash
bin$ pwd
$HOME/llvm/test/build/bin/
bin$ llc -march=cpu0 -relocation-model=pic -filetype=obj ch4_1_math_math.bc -o ch4_1_math.cpu0.o
```

Expected PDF error:

```text
~/llvm/test/build/bin/llc: target does not support generation of this file type!
```

This is the baseline before Chapter 5 object-file support is added. It is not
expected to fail once the current implementation is present.

### Inspect Existing Assembly

```bash
input$ cat ch4_1_math.cpu0.s
```

The PDF uses this only to show the assembly stream that will be encoded into an
object file.

### Big-Endian Object File

```bash
bin$ pwd
$HOME/llvm/test/build/bin/
bin$ llc -march=cpu0 -relocation-model=pic -filetype=obj ch4_1_math.bc -o ch4_1_math.cpu0.o
input$ objdump -s ch4_1_math.cpu0.o
```

PDF expectation:

```text
ch4_1_math.cpu0.o:            file format elf32-big

Contents of section .text:
 0000 09ddffc8 09200005 022d0034 09200002
```

### Little-Endian Object File

```bash
input$ ~/llvm/test/build/bin/llc -march=cpu0el -relocation-model=pic -filetype=obj ch4_1_math.bc -o ch4_1_math.cpu0el.o
input$ objdump -s ch4_1_math.cpu0el.o
```

PDF expectation:

```text
ch4_1_math.cpu0el.o:              file format elf32-little

Contents of section .text:
 0000 c8ffdd09 05002009 34002d02 02002009
```

## Workspace Test Commands

Use these commands from `/Users/jacob/workspace/compiler/llvm-project`.

### Test 1: Assembly Still Emits

```bash
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=asm /tmp/ch4_1_math.bc -o /tmp/ch4_1_math.cpu0.s
sed -n '1,80p' /tmp/ch4_1_math.cpu0.s
```

Check that the output contains `_Z9test_mathv` and Cpu0 instructions such as
`addiu`, `st`, `ld`, and `ret`.

### Test 2: Big-Endian ELF Object

```bash
$BIN/llc -march=cpu0 -relocation-model=pic -filetype=obj /tmp/ch4_1_math.bc -o /tmp/ch4_1_math.cpu0.o
$BIN/llvm-objdump -s /tmp/ch4_1_math.cpu0.o
$BIN/llvm-readobj -h /tmp/ch4_1_math.cpu0.o
```

Current expected checks:

```text
Format: elf32-cpu0
DataEncoding: BigEndian
Machine: 0x3E7

Contents of section .text:
 0000 09ddffc0 09200005 022c0000 09200002
```

The first word can differ from the PDF (`09ddffc8`) because the current backend
uses a different stack frame size for this input. The important Chapter 5 check
is that an ELF32 Cpu0 object is produced and the bytes are big-endian.

### Test 3: Little-Endian ELF Object

```bash
$BIN/llc -march=cpu0el -relocation-model=pic -filetype=obj /tmp/ch4_1_math.bc -o /tmp/ch4_1_math.cpu0el.o
$BIN/llvm-objdump -s /tmp/ch4_1_math.cpu0el.o
$BIN/llvm-readobj -h /tmp/ch4_1_math.cpu0el.o
```

Current expected checks:

```text
Format: elf32-cpu0el
DataEncoding: LittleEndian
Machine: 0x3E7

Contents of section .text:
 0000 c0ffdd09 05002009 00002c02 02002009
```

The first little-endian word (`c0ffdd09`) is the byte-reversed form of the
current big-endian word (`09ddffc0`).

### Test 4: Compare Big/Little Byte Order

```bash
$BIN/llvm-objdump -s /tmp/ch4_1_math.cpu0.o
$BIN/llvm-objdump -s /tmp/ch4_1_math.cpu0el.o
```

Expected relationship:

| Big-endian word | Little-endian word | Meaning |
|---|---|---|
| `09ddffc0` | `c0ffdd09` | same first instruction, byte-reversed |
| `09200005` | `05002009` | same second instruction, byte-reversed |
| `022c0000` | `00002c02` | same third instruction, byte-reversed |
| `09200002` | `02002009` | same fourth instruction, byte-reversed |

## Chapter 5 Coverage Map

The commands above exercise the implementation from the whole chapter:

| Section | What the test covers |
|---|---|
| 5.1 Translate into obj file | `llc -filetype=obj` succeeds for `cpu0` and `cpu0el` |
| 5.2 ELF obj related code | `Cpu0AsmBackend`, `Cpu0ELFObjectWriter`, `Cpu0MCCodeEmitter`, `Cpu0MCExpr`, and `Cpu0TargetStreamer` are linked into `LLVMCpu0Desc` |
| 5.3 Work flow | `MCInst` is encoded through `Cpu0MCCodeEmitter::encodeInstruction`, `getBinaryCodeForInstr`, `getMachineOpValue`, and `getMemEncoding` |
| 5.4 Backend Target Registration Structure | ELF streamer, asm backend, asm target streamer, and big/little MC code emitters are registered |

## Encoding Notes From The PDF

Cpu0 instructions are 32 bits. The PDF demonstrates that object bytes can be
checked by hand because the instruction fields are nibble-aligned:

```text
[31:24] opcode
[23:20] ra
[19:16] rb
[15:0]  imm16
```

For the PDF's first instruction, `addiu $sp, $sp, -56` encodes as:

```text
opcode = 0x09
ra     = 0xd
rb     = 0xd
imm16  = 0xffc8
word   = 0x09ddffc8
```

In this workspace's current output, the same shape is visible with a different
immediate:

```text
word   = 0x09ddffc0
imm16  = 0xffc0
```
