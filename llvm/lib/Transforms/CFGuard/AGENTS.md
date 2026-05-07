<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CFGuard

## Purpose

Control Flow Guard (CFG) instrumentation for Microsoft Windows targets. Adds runtime checks to prevent indirect function calls from jumping to invalid code addresses, protecting against control flow hijacking attacks.

## Key Files

| File | Description |
|------|-------------|
| `CFGuard.cpp` | Main CFG instrumentation pass |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When modifying CFGuard instrumentation:

1. Update CFGuard.cpp for core transformation logic
2. Handle both valid dispatch table construction and indirect call instrumentation
3. Add target-specific checks (Windows x86/x64 specific)
4. Test on Windows targets using llvm-lit
5. Coordinate with AsmPrinter for code generation

### Common Patterns

- **Indirect call detection**: Scan for call/invoke instructions with non-function-constant targets
- **Dispatch table building**: Create tables of valid indirect call targets
- **Runtime checks**: Insert CFG check intrinsics before indirect calls
- **Target lowering**: Lower CFG checks to machine code in AsmPrinter

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR instructions and functions
- `llvm/include/llvm/Analysis/TargetLibraryInfo.h` — target information
- `llvm/include/llvm/CodeGen/` — code generation utilities
- `llvm/lib/CodeGen/` — code generation backend

### External
- Windows SDK (for CFG constants)

<!-- MANUAL: -->
