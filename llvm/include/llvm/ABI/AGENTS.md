<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ABI (Application Binary Interface)

## Purpose

The ABI directory provides platform-specific parameter classification for function calls. Given a function signature and a target platform's calling convention, ABI code determines which parameters are passed in registers, which on the stack, and how return values are passed. This information is critical for code generation, foreign function interfaces (FFI), and optimizations that reason about call semantics.

ABI classification is platform and calling convention specific—what works on x86-64 differs from ARM or RISC-V, and each has multiple calling conventions (System V AMD64 ABI, Windows x64, etc.).

## Key Files

| File | Description |
|------|-------------|
| `Types.h` | Type classification system for ABI purposes |
| `FunctionInfo.h` | Function signature analysis and parameter classification |
| `abi-breaking.h.cmake` | CMake template for ABI stability flags |

## For AI Agents

### Working In This Directory

When working with ABI classification:

1. **Never modify ABI rules lightly**—they affect binary compatibility across libraries
2. **Test with real FFI code** (calling C libraries from LLVM-compiled code)
3. **Coordinate with backend code generation** (register allocation depends on these rules)
4. **Document platform and calling convention** when adding new ABI rules
5. **Verify alignment and padding** matches platform specifications
6. **Check for register availability** when classifying parameters
7. **Test passing structures** to ensure correct stack layout and register usage

### Common Patterns

**Classifying a function signature:**
```cpp
// Platform-specific: load ABI rules for target
const ABIInfo &ABI = getABIInfoForTarget(triple);

// Analyze function type
ABIArgInfo argInfo = ABI.classifyArgumentType(paramType);
if (argInfo.isInRegister()) {
  // Parameter passed in register
} else if (argInfo.isIndirect()) {
  // Parameter passed via pointer
}
```

**Return value classification:**
```cpp
ABIArgInfo retInfo = ABI.classifyReturnType(funcReturnType);
if (retInfo.isInRegister()) {
  // Return in register
} else if (retInfo.isStructReturn()) {
  // Return via hidden pointer parameter
}
```

## Dependencies

### Internal

- `IR/` — Type system (for analyzing function signatures)
- `ADT/` — SmallVector (for parameter lists)
- `Support/` — Casting utilities

### External

- Standard library (C++17)

## Notes

The ABI directory is small by design—platform-specific ABI implementations live in the respective target backends (e.g., `llvm/lib/Target/X86/X86ISelLowering.cpp` implements x86 ABI rules). This directory provides the abstraction and common infrastructure; concrete rules are in the backends.

<!-- MANUAL: -->
