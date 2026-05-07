<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# llvm-c (stable C API headers)

## Purpose
The stable C API for LLVM, designed for language bindings (Python, Go, Rust `llvm-sys`, OCaml, etc.) and embedding LLVM in environments where C++ ABI stability is not guaranteed. Changes here have strict backwards-compatibility requirements.

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `Transforms/` | C API for transformation passes |

## For AI Agents

### Working In This Directory
- The C API has **strong backwards-compatibility guarantees** — do not remove or change existing function signatures
- New functions should be additive only; deprecated functions are kept for several releases
- All types use opaque pointer idiom (`LLVMModuleRef`, `LLVMValueRef`, etc.) — never expose internal C++ layout
- Functions are documented in the header; keep documentation accurate

### Common Patterns
- Types are `typedef struct LLVMOpaque* LLVMFooRef;`
- Functions follow `LLVM<Object><Action>` naming: `LLVMModuleCreateWithName`, `LLVMAddFunction`
- Ownership semantics must be explicit in the documentation (caller frees vs. LLVM owns)

<!-- MANUAL: -->
