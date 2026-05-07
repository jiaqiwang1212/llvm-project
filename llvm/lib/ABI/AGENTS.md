<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ABI

## Purpose

Provides platform ABI (Application Binary Interface) classification utilities. Determines and encodes how function arguments and return values are passed on specific target platforms (register allocation, stack layout, alignment rules).

## Key Files

| File | Description |
|------|-------------|
| `FunctionInfo.cpp` | Function calling convention and parameter passing information |
| `Types.cpp` | ABI type classification and layout rules |

## For AI Agents

### Working In This Directory

1. Understand target platform calling conventions (x86-64 System V, x86-64 Microsoft, ARM, etc.)
2. Know parameter passing rules: register allocation, stack alignment, aggregate handling
3. Study ABI specifications for your target platform (AMD64 ABI, ARM EABI, PowerPC, etc.)
4. Test code generation with complex function signatures
5. Verify argument/return value classification matches platform expectations
6. Coordinate with CodeGen backend for calling convention implementation

### Key Patterns

- ABI classification drives code generation for function prologue/epilogue
- Parameters classified as integral, floating-point, or aggregate (struct/union)
- X87 vs SSE floating-point conventions vary by target
- Aggregate handling: small structs may pass in registers, large ones via pointer
- Return value classification determines register(s) for result

## Dependencies

### Internal
- Depends on: LLVM Target, LLVM IR
- Used by: CodeGen (lib/CodeGen), Frontends for correct function code generation

<!-- MANUAL: -->
