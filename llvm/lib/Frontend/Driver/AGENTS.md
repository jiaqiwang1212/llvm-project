<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend/Driver

## Purpose

Provides driver utility functions and configuration classes shared between clang and other frontends. Centralizes code generation options, target information, and compilation parameter handling.

## Key Files

| File | Description |
|------|-------------|
| `CodeGenOptions.cpp` | Code generation options configuration and management |

## For AI Agents

### Working In This Directory

1. Understand code generation options and their interactions (optimization level, target CPU, feature flags, etc.)
2. Changes here affect all frontends that consume CodeGenOptions
3. Test compilation with various option combinations
4. Maintain backward compatibility with existing option handling

### Key Patterns

- CodeGenOptions encapsulates compilation flags and target-specific settings
- Driver passes options down the compiler pipeline
- Options influence IR generation, optimization, and code generation

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM MC
- Used by: clang driver, flang driver, any frontend needing code generation configuration

<!-- MANUAL: -->
