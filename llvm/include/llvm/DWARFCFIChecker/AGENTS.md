<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWARFCFIChecker

## Purpose

Validates DWARF Call Frame Information (CFI) consistency and correctness. Analyzes CFI records against function boundaries, checks for overlapping frame ranges, and verifies register state transitions. Used by llvm-dwarfdump for debug output validation.

## Key Files

| File | Description |
|------|-------------|
| `DWARFCFIAnalysis.h` | Core CFI validation logic; rule checking, range analysis (4 KB) |
| `DWARFCFIState.h` | Register/frame state machine for CFI evaluation (1 KB) |
| `DWARFCFIFunctionFrameAnalyzer.h` | Per-function CFI analysis; frame size, register locations (1 KB) |
| `DWARFCFIFunctionFrameReceiver.h` | CFI record callback sink; processes frame data (1 KB) |
| `DWARFCFIFunctionFrameStreamer.h` | Stream-based CFI processing; incremental validation (3 KB) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding DWARF CFI format (CIE, FDE, register rules)
- Analyzing state machine rules (location expressions, expressions)
- Reviewing validation logic

**Implementation tasks:**
- Adding new CFI validation rules
- Implementing architecture-specific frame checks
- Enhancing error reporting for CFI issues

### Common Patterns

1. **CIE/FDE structure**: CIE (Common Information Entry) defines defaults; FDE (Frame Description Entry) overrides per-function
2. **Register rules**: Location rules (DW_CFA_*) specify where register values saved (stack offset, register, expression)
3. **State machine**: CFA calculation (canonical frame address) step-by-step via frame instructions
4. **Range validation**: FDE ranges checked against function boundaries from symbol table
5. **Overlaps detection**: Identifies FDE ranges that overlap (indicate debug info corruption)

### CFI Instructions

- `DW_CFA_advance_loc`: Move program counter forward
- `DW_CFA_offset`: Register saved at CFA + offset
- `DW_CFA_register`: Register value in another register
- `DW_CFA_def_cfa`: Define CFA (base address for unwinding)
- `DW_CFA_expression`: Register/CFA defined by DWARF expression

### Important Notes

- **Unwinding correctness**: Invalid CFI breaks stack unwinding (crashes debuggers, profilers)
- **Architecture-specific**: Register lists, calling conventions vary by target
- **Test coverage**: Run llvm-dwarfdump --verify on binaries during development

## Dependencies

### Internal
- `llvm/DebugInfo/DWARF/` — DWARFContext, DWARFFrame for CFI reading
- `llvm/Support/` — Error, DataExtractor

### External
- None

<!-- MANUAL: -->
