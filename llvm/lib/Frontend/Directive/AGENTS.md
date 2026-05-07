<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend/Directive

## Purpose

Provides metadata and parsing support for OpenMP and OpenACC directive infrastructure. Handles extraction, validation, and representation of parallel programming directives as structured metadata that can be consumed by IR builders and lowering passes.

## Key Files

| File | Description |
|------|-------------|
| `Spelling.cpp` | Directive spelling validation and canonicalization |

## For AI Agents

### Working In This Directory

1. Understand directive syntax from OpenMP specification (https://www.openmp.org/)
2. Use OMPIRBuilder (lib/Frontend/OpenMP/OMPIRBuilder.cpp) to construct IR for directives
3. Validate directive clauses match the specification
4. Ensure directive metadata is preserved through optimization passes

### Key Patterns

- Directives parsed from comments in source code
- Metadata stored in OMPContext or ACC context objects
- Lowering decoupled from parsing (directives → context → IR builder → LLVM IR)

## Dependencies

### Internal
- Depends on: LLVM Support, Frontend utilities
- Used by: OpenMP (lib/Frontend/OpenMP), OpenACC (lib/Frontend/OpenACC)

<!-- MANUAL: -->
