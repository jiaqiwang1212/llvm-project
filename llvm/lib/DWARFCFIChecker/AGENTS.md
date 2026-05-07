<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWARFCFIChecker

## Purpose

DWARF Call Frame Information (CFI) checker and analyzer. Validates and analyzes .eh_frame and .debug_frame sections for correctness. Detects CFI encoding errors and inconsistencies.

## Key Files

| File | Description |
|------|-------------|
| `DWARFCFIAnalysis.cpp` | CFI analysis and validation logic |
| `DWARFCFIFunctionFrameAnalyzer.cpp` | Per-function frame analysis |
| `DWARFCFIFunctionFrameStreamer.cpp` | CFI streaming and output |
| `DWARFCFIState.cpp` | CFI state machine management |
| `Registers.h` | Register information for CFI |

## For AI Agents

### Working In This Directory

1. Understand DWARF CFI format and semantics (DWARF standard, eh_frame spec)
2. Know register numbering and calling conventions
3. Test with real binaries containing .eh_frame
4. Verify frame unwinding correctness
5. Detect CFI encoding errors and malformed instructions
6. Test on multiple architectures (x86-64, ARM, PowerPC)
7. Verify stack trace generation accuracy

### Key Patterns

- CFI instructions describe how to unwind stack frames
- CIE (Common Information Entry) contains default rules
- FDE (Frame Description Entry) per-function CFI
- State machine processes CFI instructions sequentially
- Register recovery enables stack unwinding in debuggers/profilers

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM DebugInfo, DWARF utilities
- Used by: Debuggers, profilers, stack trace analysis tools

<!-- MANUAL: -->
