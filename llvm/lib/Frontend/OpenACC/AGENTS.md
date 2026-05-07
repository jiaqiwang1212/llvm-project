<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend/OpenACC

## Purpose

Provides OpenACC directive support for compiler frontends. OpenACC is a directive-based API for expressing accelerator parallelism (GPU/FPGA) at a higher level than CUDA or HIP. This module is in early development stage.

## Key Files

| File | Description |
|------|-------------|
| `ACC.cpp` | Core OpenACC directive handling and lowering |

## For AI Agents

### Working In This Directory

1. Consult OpenACC specification (https://www.openacc.org/)
2. Understand pragma syntax for parallel, kernels, data regions
3. Map directives to device operations (kernel launches, memory transfers, synchronization)
4. Test with simple OpenACC kernels before complex nested patterns
5. Note: This is early-stage; full feature coverage may be incomplete

### Key Patterns

- OpenACC directives specify parallelism and data movement
- Kernels region → GPU kernel launch
- Data region → GPU memory allocation and transfers
- Parallel region → Thread-level parallelism

## Dependencies

### Internal
- Depends on: Frontend/Directive, Frontend/Offloading, LLVM IR
- Used by: Clang with OpenACC enabled

<!-- MANUAL: -->
