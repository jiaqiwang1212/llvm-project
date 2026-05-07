<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/MC

## Purpose

Machine Code (MC) layer — LLVM's abstraction for machine code generation, assembly parsing, object file writing, and disassembly. Handles instruction representation, code emission, object file format support (ELF, Mach-O, COFF, Wasm, GOFF, SPIRV), and the assembler/disassembler interfaces.

## Key Files

| File | Category | Description |
|------|----------|-------------|
| `MCContext.cpp` | Core | Global state for MC (symbols, sections, fixups, labels) |
| `MCInst.cpp` | Core | Machine instruction representation |
| `MCInstPrinter.cpp` | Core | Base for target-specific instruction printing |
| `MCStreamer.cpp` | Streaming | Abstract streamer interface for code emission |
| `MCAsmStreamer.cpp` | Streaming | Assembly text streamer (emits .s files) |
| `MCObjectStreamer.cpp` | Streaming | Object file streamer base class |
| `MCNullStreamer.cpp` | Streaming | No-op streamer for testing |
| `MCDXContainerStreamer.cpp` | Streaming | Direct-X container output |
| `MCSPIRVStreamer.cpp` | Streaming | SPIR-V code output |
| `MCSection.cpp` | Sections | Section abstraction (code, data, BSS, custom) |
| `MCSectionMachO.cpp` | Sections | Mach-O section representation |
| `MCSymbol.cpp` | Symbols | Symbol representation (labels, extern, weak) |
| `MCSymbolELF.cpp` | Symbols | ELF-specific symbol attributes |
| `MCSymbolGOFF.cpp` | Symbols | GOFF-specific symbol attributes |
| `MCSymbolXCOFF.cpp` | Symbols | XCOFF-specific symbol attributes |
| `MCLabel.cpp` | Symbols | Label handling |
| `MCFragment.cpp` | Assembly | Fragment representation (instructions, data, alignment) |
| `MCAssembler.cpp` | Assembly | Assembler state and layout |
| `MCExpr.cpp` | Expressions | Constant expressions (arithmetic, symbols, relocations) |
| `MCValue.cpp` | Expressions | Resolved expression values |
| `MCFixup.cpp` | Relocations | Fixup representation for relocation records |
| `MCCodeEmitter.cpp` | Codegen | Abstract code emitter (base for targets) |
| `MCInstrDesc.cpp` | Instruction Info | Instruction descriptor tables |
| `MCInstrInfo.cpp` | Instruction Info | Instruction database |
| `MCRegisterInfo.cpp` | Register Info | Register names, aliases, classes |
| `MCSubtargetInfo.cpp` | Subtarget | Processor feature sets and scheduling info |
| `MCSchedule.cpp` | Scheduling | Instruction scheduling model |
| `MCInstrAnalysis.cpp` | Analysis | Instruction analysis utilities |
| `MCAsmInfo.cpp` | Target Info | Assembly syntax (directives, registers, ASM output format) |
| `MCAsmInfoCOFF.cpp` | Target Info | COFF-specific assembly syntax |
| `MCAsmInfoDarwin.cpp` | Target Info | Darwin/Mach-O-specific assembly syntax |
| `MCAsmInfoELF.cpp` | Target Info | ELF-specific assembly syntax |
| `MCAsmInfoGOFF.cpp` | Target Info | GOFF-specific assembly syntax |
| `MCAsmInfoWasm.cpp` | Target Info | Wasm-specific assembly syntax |
| `MCAsmInfoXCOFF.cpp` | Target Info | XCOFF-specific assembly syntax |
| `MCAsmMacro.cpp` | Directives | Assembly macro expansion |
| `MCObjectWriter.cpp` | Object Writer | Abstract object file writer interface |
| `ELFObjectWriter.cpp` | Object Writer | ELF object file generation (relocations, symbols, sections) |
| `MachObjectWriter.cpp` | Object Writer | Mach-O object file generation |
| `WinCOFFObjectWriter.cpp` | Object Writer | Windows COFF object file generation |
| `WasmObjectWriter.cpp` | Object Writer | WebAssembly object file generation |
| `GOFFObjectWriter.cpp` | Object Writer | z/OS GOFF object file generation |
| `SPIRVObjectWriter.cpp` | Object Writer | SPIR-V binary code generation |
| `MCELFObjectTargetWriter.cpp` | Object Writer | ELF target-specific details |
| `MCMachObjectTargetWriter.cpp` | Object Writer | Mach-O target-specific details |
| `MCWasmObjectTargetWriter.cpp` | Object Writer | Wasm target-specific details |
| `MCXCOFFObjectTargetWriter.cpp` | Object Writer | XCOFF target-specific details |
| `MCDwarf.cpp` | Debugging | DWARF debug info emission |
| `MCCodeView.cpp` | Debugging | CodeView debug info (Visual Studio) |
| `MCPseudoProbe.cpp` | Profiling | Pseudo-probe insertion for PGO |
| `MCLFI.cpp` | Exception | Local Function ID tracking |
| `MCLFIRewriter.cpp` | Exception | Local Function ID rewriting |
| `MCWin64EH.cpp` | Exception | Windows x64 exception handling |
| `MCWinEH.cpp` | Exception | General Windows exception handling |
| `MCAsmBackend.cpp` | Assembly | Abstract assembler backend (fixups, relocation) |
| `MCAsmMacro.cpp` | Assembly | Macro processing |
| `MCObjectFileInfo.cpp` | Object Info | Object file format information (section names, flags) |
| `TargetRegistry.cpp` | Registry | Target registration and lookup |
| `MCDXContainerWriter.cpp` | Object Writer | Direct-X container writer |
| `MCELFStreamer.cpp` | Streaming | ELF-specific streamer |
| `MCMachOStreamer.cpp` | Streaming | Mach-O-specific streamer |
| `MCWasmStreamer.cpp` | Streaming | Wasm-specific streamer |
| `MCWinCOFFStreamer.cpp` | Streaming | Windows COFF-specific streamer |
| `MCXCOFFStreamer.cpp` | Streaming | XCOFF-specific streamer |
| `MCGOFFStreamer.cpp` | Streaming | GOFF-specific streamer |
| `MCLinkerOptimizationHint.cpp` | Optimization | Linker optimization hints |
| `MCTargetOptions.cpp` | Options | MC-level target options |
| `MCTargetOptionsCommandFlags.cpp` | Options | Command-line flag parsing for MC |
| `ConstantPools.cpp` | Assembly | Constant pool management |
| `DXContainerPSVInfo.cpp` | Format | DirectX pipeline state validation |
| `DXContainerRootSignature.cpp` | Format | DirectX root signature handling |
| `StringTableBuilder.cpp` | Utilities | String table building for object files |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `MCDisassembler/` | Disassembler framework and base classes — see `MCDisassembler/AGENTS.md` |
| `MCParser/` | Assembly text parser (AsmParser) — see `MCParser/AGENTS.md` |

## For AI Agents

### Working In This Directory

The MC layer is the bridge between high-level IR and low-level machine code. When working here:

1. **Understand MC abstractions**: Streamers emit code, MCContext holds state, MCInst represents instructions, MCExpr represents constants.
2. **Multi-format support**: Always consider all object formats (ELF, Mach-O, COFF, Wasm, GOFF, SPIRV). Add tests for each format.
3. **Target-specific code**: MCInstrInfo, MCInstrDesc, MCSubtargetInfo are provided by target backends. Don't hardcode target details.
4. **Relocation handling**: Fixups are resolved during object file writing. Understand how targets override fixup/relocation behavior.
5. **Streaming model**: MCStreamer is abstract. Different streamers (ASM, ELF, Mach-O, etc.) implement emission differently.
6. **Symbol management**: MCContext holds symbols. Symbol resolution happens at link time, not here.

### Common Patterns

- **Instruction representation**: MCInst holds instruction opcode + operands. Use MCOperand for flexible operand handling.
- **Code emission**: Use MCStreamer interface (emitInstruction, emitLabel, emitData, emitAssignment).
- **Constant expressions**: MCExpr represents compile-time constants (arithmetic, symbols, relocations).
- **Sections**: All code/data goes into MCSection. Targets define custom sections (text, data, bss, etc.).
- **Fixups**: Record relocations with MCFixup. Let MCObjectWriter resolve them at object file writing time.
- **Target info**: Query MCInstrInfo, MCRegisterInfo, MCSubtargetInfo for target-specific details.

## Dependencies

### Internal
- `llvm/Support/` — Core utilities, error handling, I/O
- `llvm/MC/MCTargetDesc.h` — Target descriptor interfaces
- `llvm/TargetParser/` — Target feature/CPU parsing
- Target backends (llvm/Target/) — MCInstrInfo, MCRegisterInfo, MCSubtargetInfo implementations

### External
- Standard C++ library
- Platform-specific APIs (file I/O, memory)

<!-- MANUAL: -->
