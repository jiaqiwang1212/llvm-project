<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AsmPrinter

## Purpose

Converts machine IR (MachineFunction, MachineInstr) into assembly text or object code. Emits DWARF debug information. Primary output driver for the CodeGen pipeline. One AsmPrinter instance per target architecture.

## Key Files

| File | Purpose |
|------|---------|
| `AsmPrinter.cpp` | Base AsmPrinter class, instruction emission driver |
| `AsmPrinterDwarf.cpp` | DWARF-specific emission helpers |
| `AsmPrinterInlineAsm.cpp` | Inline assembly parsing and emission |
| `DwarfDebug.cpp` | DWARF debug information emission (core) |
| `DwarfDebug.h` | DwarfDebug class definition |
| `DwarfCompileUnit.cpp` | DWARF compilation unit (CU) emission |
| `DwarfCompileUnit.h` | DwarfCompileUnit class definition |
| `DwarfUnit.cpp` | Base DWARF unit abstraction |
| `DwarfUnit.h` | DwarfUnit class definition |
| `DwarfExpression.cpp` | DWARF expression encoding (variable locations) |
| `DwarfExpression.h` | DwarfExpression class definition |
| `DwarfFile.cpp` | DWARF file/directory table management |
| `DwarfFile.h` | DwarfFile class definition |
| `DIE.cpp` | DWARF information entry (DIE) representation |
| `DIEHash.cpp` | DIE signature/hash computation |
| `DIEHash.h` | DIEHash class definition |
| `EHStreamer.cpp` | Exception handling frame emission (CFI) |
| `EHStreamer.h` | EHStreamer class definition |
| `DwarfCFIException.cpp` | DWARF call frame information exception handling |
| `DwarfException.h` | Base exception handling interfaces |
| `WinException.cpp` | Windows (SEH) exception handling |
| `WinException.h` | Windows exception class definition |
| `WinCFGuard.cpp` | Windows Control Flow Guard emission |
| `WinCFGuard.h` | WinCFGuard class definition |
| `AIXException.cpp` | AIX exception handling |
| `ARMException.cpp` | ARM exception handling |
| `WasmException.cpp` | WebAssembly exception handling |
| `WasmException.h` | WasmException class definition |
| `CodeViewDebug.cpp` | CodeView (MSVC) debug information emission |
| `CodeViewDebug.h` | CodeViewDebug class definition |
| `DebugLocStream.cpp` | Debug location stream encoding |
| `DebugLocStream.h` | DebugLocStream class definition |
| `AddressPool.cpp` | DWARF address pool (.debug_addr) |
| `AddressPool.h` | AddressPool class definition |
| `DwarfStringPool.cpp` | DWARF string pool (.debug_str) |
| `DwarfStringPool.h` | DwarfStringPool class definition |
| `AccelTable.cpp` | DWARF acceleration tables (.debug_names, .debug_pubnames) |
| `ByteStreamer.h` | Raw bytes emission abstraction |
| `DbgEntityHistoryCalculator.cpp` | Debug entity location history tracking |
| `DebugHandlerBase.cpp` | Base debug handler abstraction |
| `DebugLocEntry.h` | Debug location entry representation |
| `PseudoProbePrinter.cpp` | Pseudo probe (coverage) emission |
| `PseudoProbePrinter.h` | PseudoProbePrinter class definition |
| `ErlangGCPrinter.cpp` | Erlang garbage collector metadata |
| `OcamlGCPrinter.cpp` | OCaml garbage collector metadata |

## For AI Agents

### Working In This Directory

The AsmPrinter directory handles final code emission. Start here for:
- Assembly/object code generation
- DWARF debug information (most complex part)
- Exception handling frame information
- Target-specific debug format (CodeView, etc.)

AsmPrinter is driven by `MachineFunctionPass` subclasses that call `EmitFunctionBody()`, `EmitDebugInfo()`, etc. Each target has an AsmPrinter subclass (e.g., `X86AsmPrinter`).

### Common Patterns

**DWARF Emission**: Most work centers on `DwarfDebug`. It tracks source locations, variable scopes, and emits `.debug_*` sections:
```cpp
DwarfDebug dwarf;
dwarf.beginModule(Module);
for (Function &F : Module) {
  dwarf.beginFunction(MF);
  // ... process instructions ...
  dwarf.endFunction();
}
dwarf.endModule();
```

**AsmPrinter Subclass**: Each target implements an AsmPrinter:
```cpp
class X86AsmPrinter : public AsmPrinter {
  void emitInstruction(const MachineInstr *MI) override;
  // ... other target-specific methods ...
};
```

**Exception Handling**: Different platforms use different EH formats:
- DWARF CFI (most Unix platforms)
- Windows SEH (.pdata/.xdata)
- ARM EHABI

**Debug Locations**: Variable locations tracked as `DebugLoc` objects attached to MachineInstr. AsmPrinter queries these to emit debug info.

## Dependencies

### Internal
- `../` — CodeGen core (MachineFunction, MachineInstr, LiveIntervals)
- `include/llvm/CodeGen/AsmPrinter.h` — Base AsmPrinter header
- `include/llvm/DebugInfo/DWARF/` — DWARF constants and utilities

### External
- `include/llvm/MC/` — MC layer (MCStreamer, MCSection, etc.) for actual emission
- `IR/` — LLVM IR metadata (DebugLoc, DICompileUnit, etc.)
- Each target's `Target/*/AsmPrinter/` for target-specific subclasses

<!-- MANUAL: -->
