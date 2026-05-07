<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MC

## Purpose

Machine Code abstraction layer below CodeGen. Represents executable machine code at the level of assemblers, disassemblers, object file writers, and linkers. The MC layer is independent of CodeGen: it can be used by llvm-mc (standalone assembler), disassemblers, and other tools that work with binary formats without full code generation.

## Key Files

| File | Purpose |
|------|---------|
| `MCInst.h` | Machine code instruction (opcode, operands) |
| `MCOperand.h` | Machine code operand (register, immediate, expr, etc.) |
| `MCInstrInfo.h` | Metadata about machine instructions |
| `MCInstrDesc.h` | Descriptor for a single machine instruction |
| `MCRegisterInfo.h` | Register definitions and register classes |
| `MCSubtargetInfo.h` | CPU features, scheduling information |
| `MCStreamer.h` | Abstract interface for emitting machine code |
| `MCContext.h` | Context for MC operations (symbols, sections, labels) |
| `MCAsmInfo.h` | Target-specific assembler information (syntax, directives) |
| `MCAsmBackend.h` | Target-specific assembler backend (fixups, relaxation, object emission) |
| `MCCodeEmitter.h` | Encodes MCInst to binary |
| `MCInstBuilder.h` | Helper for building MCInst objects |
| `MCInstPrinter.h` | Prints MCInst as assembly language |
| `MCSection.h` | Representation of object file section |
| `MCSymbol.h` | Label/symbol in machine code |
| `MCExpr.h` | Expression (constant, symbol reference, arithmetic) |
| `MCFixup.h` | Relocation fixup (address that needs resolution at link time) |
| `MCAssembler.h` | Assembler: manages sections, symbols, fixups |
| `MCObjectWriter.h` | Abstract object file writer |
| `MCDisassembler/` | Disassembly interface and implementations |
| `MCParser/` | Assembly language parser |
| `MCDwarf.h` | DWARF debug info encoding |
| `MCValue.h` | Value of an expression (immediate or relocation) |
| `MCLabel.h` | Labels for jump targets and debugging |
| `MCInstrAnalysis.h` | Analysis of machine code instructions |
| `MCSchedule.h` | Instruction scheduling information |
| `TargetRegistry.h` | Registry for MC components (MCInstrInfo, MCRegisterInfo factories) |

## For AI Agents

### Working In This Directory

**Understand the MC stack:**
- MCInst is the core: opcode + operands (registers, immediates, expressions)
- MCContext manages symbols, sections, labels, and global state
- MCStreamer is the output interface (abstract; subclassed for different formats)
- MCAssembler manages the assembly process: sections, symbols, fixups, relaxation
- MCCodeEmitter encodes MCInst to binary
- MCObjectWriter writes the assembled binary to object file format (ELF, Mach-O, COFF)

**Uses of MC:**
- CodeGen emits MCInst via MCStreamer
- llvm-mc assembler parses .s files and emits MCInst
- Disassemblers decode binary and produce MCInst
- Tools like llvm-objdump print MCInst via MCInstPrinter

**Target-specific MC components:**
- MCInstrInfo, MCRegisterInfo - metadata (generated from `.td` via TableGen)
- MCAsmInfo - assembler syntax (Intel vs AT&T, directive prefixes, etc.)
- MCAsmBackend - fixups, relaxation, object emission
- MCCodeEmitter - binary encoding
- MCInstPrinter - assembly output formatting
- MCDisassembler - binary decoding

### Common Patterns

**Build and emit an instruction:**
```cpp
MCContext &Ctx = ...;
MCStreamer &MS = ...;  // e.g., MCELFStreamer
MCInst Inst;
Inst.setOpcode(X86::ADD32rr);
Inst.addOperand(MCOperand::createReg(X86::EAX));
Inst.addOperand(MCOperand::createReg(X86::EBX));
MS.emitInstruction(Inst, ...);
```

**Create a symbol and section:**
```cpp
MCSymbol *Sym = Ctx.getOrCreateSymbol("function_name");
MCSection *TextSection = Ctx.getELFSection(".text", ELF::SHT_PROGBITS, ...);
MS.switchSection(TextSection);
MS.emitLabel(Sym);
```

**Relocation fixup:**
```cpp
MCFixup Fixup = MCFixup::create(Offset, Expr, FK_Data_4, SMLoc());
Fixups.push_back(Fixup);
```

**Register info query:**
```cpp
MCRegisterInfo RI(...);
unsigned RegID = RI.getDwarfRegNum(PhysReg, ...);
```

## Dependencies

### Internal
- `Support/` - data structures, error handling
- `CodeGen/` (optional) - CodeGen emits via MCStreamer; MC does not depend on CodeGen

### Format-specific
- Subclasses for ELF (MCELFStreamer, MCELFObjectWriter)
- Subclasses for Mach-O (MCMachObjectWriter)
- Subclasses for COFF (MCWinCOFFStreamer)
- Subclasses for Wasm, GOFF, SPIRV, DXContainer, etc.

<!-- MANUAL: -->
