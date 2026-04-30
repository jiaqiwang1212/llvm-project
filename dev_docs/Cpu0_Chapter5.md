# Cpu0 Backend Chapter 5: Generating Object Files

## Overview

Chapter 5 extends the Cpu0 backend from assembly-only output to ELF object-file
output. Before this chapter, `llc -filetype=obj` fails with:

```text
target does not support generation of this file type!
```

After this chapter, `llc` can produce both big-endian and little-endian ELF32
relocatable objects:

```bash
llc -march=cpu0   -relocation-model=pic -filetype=obj ch4_1_math.bc -o ch4_1_math.cpu0.o
llc -march=cpu0el -relocation-model=pic -filetype=obj ch4_1_math.bc -o ch4_1_math.cpu0el.o
```

The companion runbook with the exact PDF commands and workspace equivalents is
`test/ch5_test.md`.

## Section Coverage

| PDF section | Backend work |
|---|---|
| 5.1 Translate into obj file | Enable `llc -filetype=obj` for `cpu0` and `cpu0el` |
| 5.2 ELF obj related code | Add MC/ELF files: asm backend, object writer, code emitter, MC expr, target streamer |
| 5.3 Work flow | Encode `MCInst` into binary bytes through `Cpu0MCCodeEmitter` and TableGen-generated encoders |
| 5.4 Backend Target Registration Structure | Register ELF streamer, asm backend, target streamer, and big/little MC code emitters |

## Files Added

### `MCTargetDesc/Cpu0FixupKinds.h`

Defines target-specific fixup kinds. The enum order must match the fixup-info
table in `Cpu0AsmBackend.cpp`.

| Fixup | Relocation |
|---|---|
| `fixup_Cpu0_32` | `R_CPU0_32` |
| `fixup_Cpu0_HI16` | `R_CPU0_HI16` |
| `fixup_Cpu0_LO16` | `R_CPU0_LO16` |
| `fixup_Cpu0_GPREL16` | `R_CPU0_GPREL16` |
| `fixup_Cpu0_GOT` | `R_CPU0_GOT16` |
| `fixup_Cpu0_GOT_HI16` | `R_CPU0_GOT_HI16` |
| `fixup_Cpu0_GOT_LO16` | `R_CPU0_GOT_LO16` |

### `MCTargetDesc/Cpu0AsmBackend.h/.cpp`

Implements the target `MCAsmBackend`:

- `createObjectTargetWriter()` creates the Cpu0 ELF object writer.
- `applyFixup()` writes resolved fixup values into instruction/data bytes.
- `getFixupKindInfo()` describes target fixup bit widths.
- `writeNopData()` supplies NOP padding support.
- `createCpu0AsmBackend()` is the factory registered with `TargetRegistry`.

Modern LLVM API notes:

- `applyFixup` uses `const MCFragment &` and `uint8_t *Data`.
- `getFixupKindInfo` returns `MCFixupKindInfo` by value.
- `writeNopData` receives `const MCSubtargetInfo *`.

### `MCTargetDesc/Cpu0ELFObjectWriter.cpp`

Implements the Cpu0 `MCELFObjectTargetWriter`:

- Sets the ELF machine id to `ELF::EM_CPU0`.
- Maps each Cpu0 fixup kind to the corresponding `R_CPU0_*` relocation.
- Controls whether relocation records stay symbol-based through
  `needsRelocateWithSymbol`.

Modern LLVM API notes:

- `getRelocType` signature is `(const MCFixup &, const MCValue &, bool)`.
- `needsRelocateWithSymbol` signature is `(const MCValue &, unsigned)`.

### `MCTargetDesc/Cpu0MCCodeEmitter.h/.cpp`

Converts `MCInst` to object-code bytes:

- `encodeInstruction()` calls TableGen-generated `getBinaryCodeForInstr()`.
- `support::endian::write<uint32_t>()` writes big/little-endian instruction
  bytes.
- `getMachineOpValue()` encodes registers through
  `MCRegisterInfo::getEncodingValue`.
- `getExprOpValue()` creates relocation fixups for Cpu0 target expressions.
- `getMemEncoding()` packs `ld`/`st` memory operands.

`Cpu0InstrInfo.td` already provides the TableGen hook used by 5.3:

```tablegen
def mem : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPROut, simm16);
  let EncoderMethod = "getMemEncoding";
}
```

Modern LLVM API notes:

- `encodeInstruction` writes to `SmallVectorImpl<char> &CB`, not
  `raw_ostream &OS`.
- The generated include in this tree is `Cpu0GenCodeEmitter.inc`.

### `MCTargetDesc/Cpu0MCExpr.h/.cpp`

Adds Cpu0 target MC expressions for relocation modifiers such as `%hi`, `%lo`,
`%got`, `%got_hi`, `%got_lo`, and `%gp_rel`.

Modern LLVM API notes:

- `MCSymbolRefExpr::create(Symbol, Ctx)` uses the two-argument form.
- `MCExpr::print` is private; use `MAI->printExpr(OS, *Expr)`.
- `evaluateAsRelocatableImpl` takes `const MCAssembler *`.
- `fixELFSymbolsInTLSFixups` is no longer part of the target expression
  interface used here.

### `MCTargetDesc/Cpu0TargetStreamer.cpp` and `Cpu0TargetStreamer.h`

Adds the target streamer classes used by registration:

- `Cpu0TargetStreamer`
- `Cpu0TargetAsmStreamer`

`Cpu0TargetStreamer.h` lives in the Cpu0 target root and includes
`llvm/Support/FormattedStream.h` for `formatted_raw_ostream`.

## Files Modified

### `MCTargetDesc/CMakeLists.txt`

Adds the new Chapter 5 implementation files to `LLVMCpu0Desc`:

```cmake
Cpu0AsmBackend.cpp
Cpu0ELFObjectWriter.cpp
Cpu0MCCodeEmitter.cpp
Cpu0MCExpr.cpp
Cpu0TargetStreamer.cpp
```

### `MCTargetDesc/Cpu0BaseInfo.h`

Includes `Cpu0FixupKinds.h` so shared MC helpers can refer to Cpu0 fixups.

### `MCTargetDesc/Cpu0MCTargetDesc.h`

Declares the factories used by target registration:

```cpp
MCCodeEmitter *createCpu0MCCodeEmitterEB(const MCInstrInfo &, MCContext &);
MCCodeEmitter *createCpu0MCCodeEmitterEL(const MCInstrInfo &, MCContext &);
MCAsmBackend *createCpu0AsmBackend(const Target &, const MCSubtargetInfo &,
                                   const MCRegisterInfo &,
                                   const MCTargetOptions &);
std::unique_ptr<MCObjectTargetWriter>
createCpu0ELFObjectWriter(const Triple &);
```

### `MCTargetDesc/Cpu0MCTargetDesc.cpp`

Registers all MC pieces needed by object emission:

```cpp
TargetRegistry::RegisterELFStreamer(*T, createMCStreamer);
TargetRegistry::RegisterAsmTargetStreamer(*T, createCpu0AsmTargetStreamer);
TargetRegistry::RegisterMCAsmBackend(*T, createCpu0AsmBackend);

TargetRegistry::RegisterMCCodeEmitter(getTheCpu0Target(),
                                      createCpu0MCCodeEmitterEB);
TargetRegistry::RegisterMCCodeEmitter(getTheCpu0elTarget(),
                                      createCpu0MCCodeEmitterEL);
```

Modern LLVM API notes:

- `RegisterELFStreamer` callback no longer receives `RelaxAll`.
- `RegisterAsmTargetStreamer` callback no longer receives `isVerboseAsm`.
- `RegisterMCCodeEmitter` callback no longer receives `MCRegisterInfo &MRI`.

## Object Emission Flow

Chapter 5's key runtime path is:

```text
Cpu0AsmPrinter::emitInstruction(MachineInstr)
  -> Cpu0MCInstLower::Lower(MachineInstr -> MCInst)
  -> MCObjectStreamer::emitInstruction(MCInst)
  -> MCELFStreamer::emitInstToData(MCInst)
  -> Cpu0MCCodeEmitter::encodeInstruction(MCInst)
  -> getBinaryCodeForInstr(MCInst)       [TableGen-generated]
  -> getMachineOpValue / getMemEncoding  [target hooks]
  -> ELFObjectWriter::writeObject()
```

When `-filetype=asm` is used, the streamer is an assembly streamer. When
`-filetype=obj` is used, the driver creates an object streamer backed by the ELF
streamer registered in `Cpu0MCTargetDesc.cpp`.

## Verification

Build:

```bash
cmake --build build --target LLVMCpu0Desc
cmake --build build --target llc
```

Create the Chapter 5 input bitcode:

```bash
build/bin/clang -target mips-unknown-linux-gnu \
  -c /Users/jacob/workspace/compiler/llvm_backend_reference/lbdex/input/ch4_1_math.cpp \
  -emit-llvm -o /tmp/ch4_1_math.bc
```

Generate and inspect big-endian object output:

```bash
build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj \
  /tmp/ch4_1_math.bc -o /tmp/ch4_1_math.cpu0.o
build/bin/llvm-objdump -s /tmp/ch4_1_math.cpu0.o
build/bin/llvm-readobj -h /tmp/ch4_1_math.cpu0.o
```

Expected current checks:

```text
Format: elf32-cpu0
DataEncoding: BigEndian
Machine: 0x3E7
Contents of section .text:
 0000 09ddffc0 09200005 022c0000 09200002
```

Generate and inspect little-endian object output:

```bash
build/bin/llc -march=cpu0el -relocation-model=pic -filetype=obj \
  /tmp/ch4_1_math.bc -o /tmp/ch4_1_math.cpu0el.o
build/bin/llvm-objdump -s /tmp/ch4_1_math.cpu0el.o
build/bin/llvm-readobj -h /tmp/ch4_1_math.cpu0el.o
```

Expected current checks:

```text
Format: elf32-cpu0el
DataEncoding: LittleEndian
Machine: 0x3E7
Contents of section .text:
 0000 c0ffdd09 05002009 00002c02 02002009
```

The PDF shows `09ddffc8` and `c8ffdd09` for the first instruction. The current
tree emits `09ddffc0` and `c0ffdd09` because the frame size for this input is
currently different. The Chapter 5 invariant is the object-file support and the
big/little-endian byte relationship.
