<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend/OpenMP

## Purpose

Provides OpenMP code generation support and IR building infrastructure. Contains the OMPIRBuilder (primary IR construction tool), directive parsing, context management, and lowering from directives to LLVM IR for parallel execution.

## Key Files

| File | Description |
|------|-------------|
| `OMPIRBuilder.cpp` | Core OpenMP IR builder; constructs LLVM IR for parallel regions, tasks, synchronization |
| `OpenMPToLLVMIRTranslation.cpp` | Translation from OpenMP AST to LLVM IR |
| `OMPContext.cpp` | OpenMP execution context management |
| `DirectiveNameParser.cpp` | OpenMP directive syntax parsing |
| `OMP.cpp` | Main OpenMP support utilities |

## For AI Agents

### Working In This Directory

1. Primary entry point is OMPIRBuilder for IR construction
2. Study OpenMP specification (https://www.openmp.org/) for directive semantics
3. Understand threaded execution models: fork-join parallelism, work distribution, synchronization
4. Test parallel regions, tasks, reductions, critical sections, barriers
5. Verify code generation for various clause combinations
6. Consider interaction with offloading (device directives)

### Key Patterns

- OMPIRBuilder constructs IR incrementally for parallel constructs
- Directives specify work sharing (parallel for, sections), synchronization (barrier, critical)
- Reductions require special lowering with atomic operations or locks
- Task dependencies tracked through task scheduling
- Target offload directives interact with Offloading infrastructure

## Dependencies

### Internal
- Depends on: LLVM IR (lib/IR), CodeGen (lib/CodeGen), Frontend/Directive, Frontend/Offloading
- Used by: clang, flang, any frontend supporting OpenMP

<!-- MANUAL: -->
