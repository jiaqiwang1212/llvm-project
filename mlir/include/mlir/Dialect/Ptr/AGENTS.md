<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Ptr Dialect

## Purpose
Provides a dialect-agnostic pointer type (`ptr.ptr<memory_space>`) and memory ops (load, store, GEP) that are independent of the LLVM dialect. Enables pointer-based IR that can be lowered to LLVM or other targets via PtrToLLVM.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, attribute, and interface definitions (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `ptr.ptr<#ptr.generic>` is the untyped pointer; memory spaces use attribute interfaces
- Op names follow `ptr.*` convention
- Lowered to `llvm.ptr` via PtrToLLVM conversion

### Common Patterns
- Memory space interfaces allow backends to extend pointer semantics for their address spaces

## Dependencies
- Depends on: LLVMIR dialect (for lowering)

<!-- MANUAL: -->
