<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Ptr Dialect

## Purpose
Implements the Ptr dialect — a typed pointer abstraction for MLIR. Provides `!ptr.ptr<memory_space>` as a portable pointer type with memory space attributes, enabling explicit pointer arithmetic and memory operations decoupled from LLVM's opaque pointer model.

## Key Files
| File | Description |
|------|-------------|
| `IR/PtrDialect.cpp` | Dialect registration |
| `IR/PtrTypes.cpp` | `!ptr.ptr<T>` type implementation |
| `IR/PtrAttrs.cpp` | Pointer attribute definitions |
| `IR/MemorySpaceInterfaces.cpp` | Memory space interface implementations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Pointer type, attributes, memory space interfaces |

## For AI Agents

### Working In This Directory
- `!ptr.ptr<memory_space>` is distinct from `!llvm.ptr` — it carries a memory space attribute for multi-address-space targets.
- `MemorySpaceInterfaces.cpp` defines the interface that memory space attributes must implement for pointer ops to query properties.
- Lowering to LLVM IR is handled by `PtrToLLVM` conversion.
- No transforms defined here — pointer manipulation transforms are in `PtrToLLVM`.

### Common Patterns
- Memory space attributes implement `MemorySpaceAttrInterface` to provide address space integers and properties.
- Pointer type builder: `PtrType::get(ctx, memorySpace)`.

## Dependencies
- `mlir/IR`, `mlir/Dialect/LLVMIR` (for lowering)

<!-- MANUAL: -->
