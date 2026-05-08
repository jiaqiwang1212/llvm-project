<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib

## Purpose
Root implementation directory for the MLIR compiler framework. Contains all C++ source files that implement the MLIR core IR, analyses, transformations, dialect infrastructure, and tooling. This is the primary build artifact source tree; headers live in `mlir/include/`.

## Key Files
| File | Description |
|------|-------------|
| `RegisterAllDialects.cpp` | Registers every bundled MLIR dialect into an `MLIRContext`; used by tools that need the full dialect set |
| `RegisterAllExtensions.cpp` | Registers all dialect extensions (interface attachments, external models) for the bundled dialects |
| `RegisterAllPasses.cpp` | Registers all bundled transformation and conversion passes into the global pass registry |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core IR data structures: operations, types, attributes, regions, blocks, values (see `IR/AGENTS.md`) |
| `Interfaces/` | Op/type/attribute interface implementations (see `Interfaces/AGENTS.md`) |
| `Analysis/` | Standalone analyses: alias analysis, dataflow, liveness, Presburger math (see `Analysis/AGENTS.md`) |
| `Transforms/` | Dialect-independent transformation passes: CSE, inliner, canonicalizer, SCCP, etc. (see `Transforms/AGENTS.md`) |
| `Pass/` | Pass manager infrastructure: scheduling, crash recovery, statistics, timing (see `Pass/AGENTS.md`) |
| `Parser/` | Textual MLIR assembly parser (see `Parser/AGENTS.md`) |
| `AsmParser/` | Low-level assembly parser primitives: lexer, token kinds, attribute/type parsing (see `AsmParser/AGENTS.md`) |
| `Bytecode/` | Binary bytecode format reader and writer (see `Bytecode/AGENTS.md`) |
| `CAPI/` | Stable C API wrapping the C++ MLIR API (see `CAPI/AGENTS.md`) |
| `Bindings/` | Language bindings — currently Python via pybind11 (see `Bindings/AGENTS.md`) |
| `Debug/` | Action-based debugging infrastructure, breakpoint managers, execution context hooks (see `Debug/AGENTS.md`) |
| `ExecutionEngine/` | JIT execution engine, runtime utilities, GPU/accelerator runtime wrappers (see `ExecutionEngine/AGENTS.md`) |
| `Query/` | `mlir-query` tool internals: matcher DSL, query parsing (see `Query/AGENTS.md`) |
| `Reducer/` | `mlir-reduce` IR minimization infrastructure (see `Reducer/AGENTS.md`) |
| `Remark/` | Optimization remark / diagnostic streaming (see `Remark/AGENTS.md`) |
| `Rewrite/` | Pattern rewrite engine: PDL bytecode interpreter, frozen pattern sets, pattern applicator (see `Rewrite/AGENTS.md`) |
| `Support/` | Low-level MLIR-specific utilities: StorageUniquer, TypeID, timing, file utilities (see `Support/AGENTS.md`) |
| `TableGen/` | TableGen backends used to generate MLIR dialect C++ from `.td` files (see `TableGen/AGENTS.md`) |
| `Target/` | Translation targets: LLVM IR, SPIR-V, C++, WebAssembly, SMTLIB (see `Target/AGENTS.md`) |
| `Tools/` | Driver implementations for `mlir-opt`, `mlir-translate`, LSP servers, `mlir-tblgen`, etc. (see `Tools/AGENTS.md`) |
| `Dialect/` | Per-dialect implementations (Affine, Arith, Linalg, LLVMIR, SPIRV, …) (see `Dialect/AGENTS.md`) |
| `Conversion/` | Dialect-to-dialect lowering passes (e.g., ArithToLLVM, SCFToControlFlow) (see `Conversion/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Most source files are generated indirectly from TableGen (`.td`) files in `mlir/include/mlir/`. Running `cmake --build <build_dir> --target mlir-headers` regenerates the `.h.inc` / `.cpp.inc` files before editing downstream `.cpp` files.
- `RegisterAll*.cpp` files must be updated when a new dialect, extension, or pass is added to the bundled set.
- Do not add dialect-specific logic here; it belongs under `Dialect/` or `Conversion/`.

### Common Patterns
- Source files `#include` generated `.inc` files (e.g., `#include "mlir/Dialect/Foo/IR/FooDialect.cpp.inc"`).
- Dialect registration uses `context.loadDialect<FooDialect>()` or `registry.insert<FooDialect>()`.
- Pass registration uses `registerFooPass()` / `registerFooPasses()` free functions declared in `.td`-generated headers.

## Dependencies

### Internal
- `mlir/include/mlir/` — all public headers and TableGen-generated `.inc` files
- `mlir/lib/IR/` — foundational; nearly everything depends on this

### External
- `llvm/lib/Support` — `llvm::StringRef`, `llvm::SmallVector`, ADT, file I/O
- `llvm/lib/TableGen` — TableGen record model (for `TableGen/` backends)

<!-- MANUAL: -->
