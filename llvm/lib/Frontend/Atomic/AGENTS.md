<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend/Atomic

## Purpose

Provides atomic operation lowering helpers and utilities for compiler frontends. Handles the transformation and optimization of atomic operations (memory synchronization primitives) from high-level language constructs into LLVM IR representations.

## Key Files

| File | Description |
|------|-------------|
| `Atomic.cpp` | Core atomic operation lowering implementation |

## For AI Agents

### Working In This Directory

1. Understand the atomic operation semantics being lowered from the source language
2. Consult LLVM's atomic operation model in lib/IR/InstrTypes.cpp and lib/CodeGen/AtomicExpandPass.cpp
3. Ensure memory ordering constraints are properly preserved during lowering
4. Test edge cases: memory barriers, fence operations, compare-and-swap variants

### Key Patterns

- Atomic operations map to LLVM IR AtomicCmpXchgInst, AtomicRMWInst, or FenceInst
- Memory ordering follows C++ atomic semantics (relaxed, release, acquire, acq_rel, seq_cst)
- Platform-specific expansion handled by backend AtomicExpandPass

## Dependencies

### Internal
- Depends on: LLVM IR (lib/IR), CodeGen (lib/CodeGen)
- Used by: Frontends that expose atomic operations (clang, flang)

<!-- MANUAL: -->
