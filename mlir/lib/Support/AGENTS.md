<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Support

## Purpose
Implements MLIR-specific low-level support utilities that are used throughout the framework but do not depend on the IR. This is the lowest layer of the MLIR library stack above LLVM Support.

## Key Files
| File | Description |
|------|-------------|
| `StorageUniquer.cpp` | `StorageUniquer`: generic uniquing allocator for `TypeStorage` and `AttributeStorage` objects; uses a `DenseMap` keyed by storage parameters within a `MLIRContext`-owned allocator |
| `TypeID.cpp` | `TypeID`: runtime type identifier for MLIR types, attributes, passes, and interfaces; provides a stable unique pointer-sized ID without RTTI |
| `InterfaceSupport.cpp` | `InterfaceMap`: maps `TypeID` → interface implementation pointer; powers the per-op/type/attribute interface lookup |
| `Timing.cpp` | `DefaultTimingManager` and `TimingScope`: hierarchical wall-clock timer used by the pass manager's `--mlir-timing` instrumentation |
| `FileUtilities.cpp` | `openInputFile()`, `openOutputFile()`: wrappers for opening files with MLIR-style diagnostics |
| `RawOstreamExtras.cpp` | Additional `llvm::raw_ostream` utilities used by MLIR's printer and diagnostic system |
| `StateStack.cpp` | `StateStack<T>`: a stack of rewriter/transformation state frames used during nested pattern application |
| `ToolUtilities.cpp` | `splitAndProcessBuffer()`: splits a `// -----`-delimited test file into chunks; used by `mlir-opt` for split-input-file mode |

## For AI Agents

### Working In This Directory
- `StorageUniquer` is fundamental to type/attribute uniquing in `MLIRContext`; changes require careful synchronization with the context's threading model.
- `TypeID` relies on static storage addresses for uniqueness; never copy or serialize `TypeID` values across process boundaries.
- `InterfaceSupport.cpp` is called on every interface query (`op.dyn_cast<MyInterface>()`); it must remain fast.
- `ToolUtilities.cpp`'s `splitAndProcessBuffer` is used by almost every `mlir-opt` integration test.

### Common Patterns
- New MLIR-wide utilities that are used by both `IR/` and `Pass/` belong here, not in either of those directories.
- Timing scopes nest automatically via RAII; `TimingScope::nest("name")` creates a child timer.

## Dependencies

### Internal
- No mlir/lib/ dependencies; this is the foundation layer

### External
- `llvm/lib/Support` — `llvm::DenseMap`, `llvm::BumpPtrAllocator`, `llvm::raw_ostream`, threading primitives

<!-- MANUAL: -->
