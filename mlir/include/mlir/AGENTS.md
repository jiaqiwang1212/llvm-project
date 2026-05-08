<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/

## Purpose
Root of the MLIR C++ public header tree. Contains all headers for the core MLIR framework including the IR, passes, transforms, analysis, interfaces, dialects, and tooling infrastructure. This directory is the primary include path added by CMake consumers of MLIR libraries.

## Key Files
| File | Description |
|------|-------------|
| `InitAllDialects.h` | Registers all bundled MLIR dialects into an MLIRContext |
| `InitAllExtensions.h` | Registers all dialect extensions |
| `InitAllPasses.h` | Registers all bundled MLIR passes |
| `InitAllTranslations.h` | Registers all bundled translation functions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core IR types: operations, types, attributes, regions, blocks, values (see `IR/AGENTS.md`) |
| `Interfaces/` | Op/type/attribute interface definitions (.td and .h) (see `Interfaces/AGENTS.md`) |
| `Analysis/` | Analyses: alias analysis, dataflow framework, Presburger math (see `Analysis/AGENTS.md`) |
| `Transforms/` | General-purpose transformation passes (see `Transforms/AGENTS.md`) |
| `Pass/` | Pass manager infrastructure and pass base classes (see `Pass/AGENTS.md`) |
| `Parser/` | High-level MLIR text format parser entry point (see `Parser/AGENTS.md`) |
| `AsmParser/` | Assembly parser interface and code-completion support (see `AsmParser/AGENTS.md`) |
| `Bytecode/` | Bytecode serialization reader/writer (see `Bytecode/AGENTS.md`) |
| `CAPI/` | Internal helpers for the C API implementation (see `CAPI/AGENTS.md`) |
| `Bindings/` | Language binding support (Python via nanobind) (see `Bindings/AGENTS.md`) |
| `Debug/` | Action-based debugging, breakpoints, execution context (see `Debug/AGENTS.md`) |
| `ExecutionEngine/` | JIT execution engine and runtime utilities (see `ExecutionEngine/AGENTS.md`) |
| `Query/` | mlir-query tool matchers and query session (see `Query/AGENTS.md`) |
| `Reducer/` | mlir-reduce test case reduction framework (see `Reducer/AGENTS.md`) |
| `Remark/` | Optimization remark streaming (see `Remark/AGENTS.md`) |
| `Rewrite/` | Pattern rewrite engine primitives (see `Rewrite/AGENTS.md`) |
| `Support/` | Low-level utilities: TypeID, LogicalResult, StorageUniquer (see `Support/AGENTS.md`) |
| `TableGen/` | C++ TableGen backend helpers for mlir-tblgen (see `TableGen/AGENTS.md`) |
| `Target/` | Translation targets: LLVM IR, SPIR-V, C++, WASM, SMTLIB (see `Target/AGENTS.md`) |
| `Tools/` | Tool main-entry-point headers: mlir-opt, mlir-translate, LSP servers (see `Tools/AGENTS.md`) |
| `Config/` | CMake-generated configuration header (see `Config/AGENTS.md`) |
| `Dialect/` | Per-dialect headers for all bundled dialects (see `Dialect/AGENTS.md`) |
| `Conversion/` | Dialect-to-dialect conversion pass headers (see `Conversion/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Do not add files directly here; use the appropriate subdirectory.
- `InitAll*.h` files must be kept in sync when adding new dialects, passes, or translations.
- Changes to `IR/` headers have the highest blast radius — many components depend on them.

### Common Patterns
- Headers use `#pragma once`.
- TableGen-generated includes follow the pattern `#include "mlir/IR/SomeOps.h.inc"`.
- Forward declarations from `mlir/IR/` are re-exported transitively; prefer minimal includes.

## Dependencies

### Internal
- `mlir/Support/` (foundational utilities used everywhere)
- `mlir/IR/` (core IR used by nearly all other subdirs)

### External
- `llvm/ADT/`, `llvm/Support/` (LLVM data structures and utilities)

<!-- MANUAL: -->
