<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Support/

## Purpose
Foundational utility headers for MLIR. Provides core primitives that are used throughout the entire MLIR codebase: `LogicalResult`, `TypeID`, `StorageUniquer`, interface support infrastructure, thread-local caches, timing utilities, and various ADT extensions. These headers have minimal dependencies and are safe to include from almost anywhere.

## Key Files
| File | Description |
|------|-------------|
| `LogicalResult.h` | `LogicalResult`, `success()`, `failure()`, `succeeded()`, `failed()` |
| `TypeID.h` | `TypeID` — lightweight unique type identifier used for interface and dialect lookup |
| `StorageUniquer.h` | `StorageUniquer` — uniquing allocator for type and attribute storage objects |
| `InterfaceSupport.h` | `InterfaceMap`, `Interface<>` CRTP base — implements the interface dispatch table |
| `LLVM.h` | Re-exports commonly used LLVM ADT types into the `mlir` namespace |
| `WalkResult.h` | `WalkResult` — control flow for `op.walk()` callbacks (advance/interrupt/skip) |
| `IndentedOstream.h` | `IndentedOstream` — wraps `raw_ostream` with indentation tracking |
| `FileUtilities.h` | `openInputFile()`, `openOutputFile()` with error reporting |
| `ToolUtilities.h` | `splitAndProcessBuffer()` for split-file test utilities |
| `Timing.h` | `TimingScope`, `DefaultTimingManager` — hierarchical timing for passes |
| `ThreadLocalCache.h` | `ThreadLocalCache<T>` — per-thread cached values |
| `StorageUniquer.h` | Uniquing storage for types and attributes |
| `DebugStringHelper.h` | `debugString(val)` — dumps any MLIR object to a string for debugging |
| `RawOstreamExtras.h` | Extra `raw_ostream` utilities |
| `ADTExtras.h` | Additional ADT utilities not in LLVM (e.g., `CopyOnWriteArrayRef`) |
| `CyclicReplacerCache.h` | Cache for cyclic type/attribute replacements |
| `StateStack.h` | `StateStack<T>` — push/pop state for recursive algorithms |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `LogicalResult` is the standard MLIR error type — use it instead of `bool` for fallible operations.
- `TypeID` is used extensively for interface lookup and dialect registration; never forge a TypeID.
- `LLVM.h` is the recommended way to get `StringRef`, `SmallVector`, `ArrayRef`, etc. in MLIR code.
- `StorageUniquer` is an internal detail of the type/attribute system; rarely needed directly.
- `InterfaceSupport.h` is the machinery behind all `OpInterface`/`TypeInterface` dispatch.

### Common Patterns
- `return success()` / `return failure()` for `LogicalResult`-returning functions.
- `if (failed(doSomething())) return failure();` for error propagation.
- `TypeID::get<MyType>()` for stable type identity.

## Dependencies

### Internal
- (minimal — this is the foundation layer)

### External
- `llvm/ADT/` (re-exported via `LLVM.h`)
- `llvm/Support/` (raw_ostream, SourceMgr)

<!-- MANUAL: -->
