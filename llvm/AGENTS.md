<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# llvm

## Purpose
The LLVM core library — a collection of reusable compiler infrastructure components including the IR, optimizer, code generator, and target backends. This is the heart of the LLVM project; tools like `clang`, `opt`, `llc`, and `lld` are built on top of it.

## Key Files

| File | Description |
|------|-------------|
| `CMakeLists.txt` | Main build configuration for the llvm library |
| `CMakePresets.json` | Common preset build configurations |
| `Maintainers.md` | Component ownership and maintainer contacts |
| `README.txt` | Brief LLVM overview and build pointer |
| `CREDITS.TXT` | Historical contributor credits |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `include/` | Public C++ headers and C API headers (see `include/AGENTS.md`) |
| `lib/` | Implementation source files (see `lib/AGENTS.md`) |
| `tools/` | Standalone executables (opt, llc, llvm-as, etc.) |
| `test/` | FileCheck-based regression tests |
| `unittests/` | GoogleTest unit tests |
| `utils/` | Developer utilities, TableGen, update scripts |
| `docs/` | Sphinx documentation source |
| `examples/` | Example passes and tools |
| `benchmarks/` | Micro-benchmarks using Google Benchmark |
| `bindings/` | Language bindings (OCaml, Go) |
| `cmake/` | CMake modules and config files |
| `resources/` | Resource files (e.g., Windows manifests) |
| `projects/` | Optional subprojects hooked into the build |
| `runtimes/` | Runtime libraries (compiler-rt, libcxx, etc.) |

## For AI Agents

### Working In This Directory
- Never edit generated files (`.inc`, `.def`) directly — regenerate from TableGen sources using `llvm-tblgen`
- Build with `cmake -G Ninja -DLLVM_TARGETS_TO_BUILD=<target> ..` from a `build/` directory outside the source tree
- The `utils/update_*.py` scripts auto-update CHECK lines in test files

### Testing Requirements
- Unit tests: `ninja check-llvm-unit` (or `check-llvm` for all including FileCheck)
- Target-specific: `ninja check-llvm-codegen-<target>`
- Run `ninja llvm-tblgen` before building if TableGen files changed

### Common Patterns
- TableGen (`.td` files) generates instruction definitions, register info, and DAG patterns
- Passes are registered via `PassRegistry` or the new `PassManager` infrastructure
- Headers in `include/llvm/` expose the public API; `lib/` contains implementations

## Dependencies

### Internal
- All components depend on `lib/Support` (basic utilities)
- CodeGen depends on `MC` (assembler) and `Target` (target description)
- IR depends on `Support` and `ADT`

### External
- zlib (optional compression)
- libxml2 (optional, for PDB/DWARF)
- Python 3 (for test runner `lit` and utility scripts)
- CMake 3.20+, Ninja or Make

<!-- MANUAL: -->
