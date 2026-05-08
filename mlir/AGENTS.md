<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir

## Purpose
MLIR (Multi-Level Intermediate Representation) is a reusable compiler infrastructure within LLVM. It provides a flexible type system, extensible dialects, and a progressive lowering framework enabling domain-specific abstractions to be compiled efficiently to hardware targets. MLIR serves as the backbone for ML compilers (TensorFlow, JAX, PyTorch), HPC frameworks, and hardware design tools.

## Key Files
| File | Description |
|------|-------------|
| `CMakeLists.txt` | Top-level CMake build entry point for all MLIR targets |
| `README.md` | Project overview and quick-start guide |
| `Maintainers.md` | List of subsystem maintainers and review areas |
| `LICENSE.TXT` | Apache 2.0 with LLVM exception license |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `include/` | Public C++ headers and TableGen definitions (see `include/AGENTS.md`) |
| `lib/` | All implementation source files (see `lib/AGENTS.md`) |
| `tools/` | CLI tools: mlir-opt, mlir-translate, mlir-lsp-server, mlir-pdll, etc. |
| `test/` | Lit-based integration test suite mirroring lib/ structure |
| `unittests/` | GoogleTest unit tests for core subsystems |
| `docs/` | English Markdown documentation and dialect references |
| `doc_zh/` | Chinese translations of the docs/ tree |
| `python/` | Python bindings (mlir Python package via pybind11/nanobind) |
| `examples/` | Standalone tutorial projects: Toy language, Transform dialect, standalone dialect |
| `benchmark/` | Microbenchmarks using Google Benchmark |
| `cmake/` | CMake modules for building MLIR-based projects |
| `utils/` | Editor integrations (vim, emacs, vscode), GDB/LLDB scripts, Jupyter support |

## For AI Agents

### Working In This Directory
- MLIR is structured around **dialects** — each dialect lives under `include/mlir/Dialect/<Name>/` (headers/TableGen) and `lib/Dialect/<Name>/` (implementation).
- TableGen (`.td` files) generates large amounts of C++ boilerplate; never edit generated files under `build/`.
- The `include/` and `lib/` trees are mirrors — changes to a dialect interface usually require touching both.
- Use `mlir-opt` (built to `build/bin/mlir-opt`) to run FileCheck-based tests: `mlir-opt --<pass> test/file.mlir | FileCheck test/file.mlir`.

### Testing Requirements
- Integration tests: `llvm-lit mlir/test/` or a specific subdirectory
- Unit tests: `ninja MLIRUnitTests` then `mlir/unittests/<Target>Tests`
- Python binding tests: `python/tests/`
- Add FileCheck tests for any new pass or op transformation

### Common Patterns
- New dialect: add `include/mlir/Dialect/<Name>/`, `lib/Dialect/<Name>/`, register in `InitAllDialects.h`
- New pass: add to `include/mlir/Transforms/` or dialect-specific `Transforms/`, register in `InitAllPasses.h`
- TableGen ODS op definition → `include/mlir/Dialect/<Name>/<Name>Ops.td`
- Conversion passes follow `lib/Conversion/<SrcDialect>To<DstDialect>/` naming

## Dependencies

### Internal (LLVM project)
- `llvm/include/llvm/` — ADT, Support, IR types used throughout
- `llvm/lib/` — LLVM backend integration via `Target/` and `ExecutionEngine/`

### External
- TableGen — code generation for ops, types, interfaces, passes
- pybind11/nanobind — Python bindings
- Google Benchmark — benchmark infrastructure

<!-- MANUAL: -->
