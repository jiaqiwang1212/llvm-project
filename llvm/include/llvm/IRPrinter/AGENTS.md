<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IRPrinter

## Purpose

The IRPrinter directory contains pass wrappers that print IR (Module, Function, or Loop) to output streams at different points in the compilation pipeline. Used for debugging, logging IR transformations, and verifying optimization effects.

## Key Files

| File | Description |
|------|-------------|
| `IRPrintingPasses.h` | Pass wrappers for IR printing (PrintModulePass, PrintFunctionPass, PrintLoopPass) |

## Subdirectories (if applicable)

No subdirectories; single header file contains all printing pass definitions.

## For AI Agents

### Working In This Directory

When using IR printing passes:

1. **Choose the right pass** — `PrintModulePass` prints entire module, `PrintFunctionPass` per-function, `PrintLoopPass` per-loop
2. **Integrate into pipeline** — Add printing passes between other passes to observe IR evolution
3. **Specify output stream** — Passes write to provided `raw_ostream` (stdout, file, StringRef, etc.)
4. **Banner text is helpful** — Passes print a banner before IR; customize it to identify the point in pipeline
5. **No IR modification** — Printing passes are read-only; they preserve all analyses

### Common Patterns

- **Before/after passes** — Add print pass before and after optimization to compare IR
- **Debug flag control** — Printing often controlled by command-line flags (e.g., `-print-after-all`)
- **Pipeline instrumentation** — StandardInstrumentations may automatically insert print passes based on configuration
- **Custom streams** — Redirect output to file, string, or custom sink via raw_ostream
- **Metadata inclusion** — Printed IR includes all metadata, attributes, and debug information

## Dependencies

### Internal

- `llvm/IR/` — IR classes (Module, Function, Loop, Instruction, Value)
- `llvm/Passes/` — New Pass Manager infrastructure (ModulePass, FunctionPass, LoopPass)
- `llvm/Support/` — raw_ostream and output formatting utilities
- `llvm/ADT/` — Data structures

### External

- Output stream is provided by caller (can be any raw_ostream subclass)

<!-- MANUAL: -->
