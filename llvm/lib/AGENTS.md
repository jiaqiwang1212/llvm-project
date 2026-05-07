<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib

## Purpose
Implementation source files for the LLVM core library. Mirrors the `include/llvm/` structure — each subdirectory here corresponds to the same-named header directory. Built as a set of static or shared libraries consumed by LLVM tools.

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `ABI/` | Platform ABI lowering utilities |
| `Analysis/` | IR analysis passes (alias analysis, CFG, loop analysis, etc.) |
| `AsmParser/` | LLVM IR textual assembly parser |
| `BinaryFormat/` | Binary format parsers (ELF, COFF, MachO, Wasm headers/enums) |
| `Bitcode/` | LLVM bitcode reader and writer |
| `Bitstream/` | Low-level bitstream container format reader |
| `CAS/` | Content-addressable storage for caching |
| `CGData/` | Code generation data collection |
| `CodeGen/` | Target-independent code generation (ISel, RA, scheduling, etc.) |
| `CodeGenTypes/` | Type lowering from IR types to machine types |
| `DebugInfo/` | DWARF, CodeView, PDB debug info reading/writing |
| `Debuginfod/` | Debuginfod HTTP client |
| `Demangle/` | C++/Rust/D symbol demangling |
| `DTLTO/` | Distributed ThinLTO support |
| `DWARFCFIChecker/` | DWARF CFI consistency checker |
| `DWARFLinker/` | DWARF debug info linker (classic and parallel) |
| `DWP/` | DWARF package file writer |
| `ExecutionEngine/` | JIT execution engines (ORC, MCJIT, Interpreter) |
| `Extensions/` | Optional LLVM extension hooks |
| `FileCheck/` | FileCheck pattern-matching engine |
| `Frontend/` | Language-agnostic frontend helpers (OpenMP, OpenACC, HLSL, offloading) |
| `Fuzzer/` | LibFuzzer implementation |
| `FuzzMutate/` | IR mutation for fuzzing |
| `HTTP/` | HTTP client utilities |
| `InterfaceStub/` | ELF interface stub generator |
| `IR/` | Core LLVM IR (Module, Function, Instruction, Type, etc.) |
| `IRPrinter/` | IR pretty-printing passes |
| `IRReader/` | Unified IR reader (bitcode + textual) |
| `LineEditor/` | Interactive line editing with history |
| `Linker/` | IR-level linker for LTO |
| `LTO/` | Link-time optimization pipeline |
| `MC/` | Machine code layer (assembler, disassembler, streamer) |
| `MCA/` | Machine code analyzer (throughput simulation) |
| `ObjCopy/` | Object file copying and transformation |
| `Object/` | Object file reading (ELF, COFF, MachO, Wasm, archive) |
| `ObjectYAML/` | YAML ↔ object file conversion |
| `Option/` | Command-line option parsing (TableGen-driven) |
| `Passes/` | Pass manager infrastructure and pipeline builders |
| `Plugins/` | Dynamic pass plugin loading |
| `ProfileData/` | Profile data reading/writing (instrumentation, sampling) |
| `Remarks/` | Optimization remark serialization |
| `SandboxIR/` | Experimental sandboxed IR for safe transformations |
| `Support/` | Core utilities (file system, memory, threads, strings, etc.) |
| `TableGen/` | TableGen language parser and backends |
| `Target/` | Target-specific backends (AArch64, X86, Mips, Cpu0, etc.) |
| `TargetParser/` | Target triple, CPU feature string parsing |
| `Telemetry/` | Telemetry/instrumentation framework |
| `Testing/` | Test support utilities |
| `TextAPI/` | Apple TBD/TextAPI file format |
| `ToolDrivers/` | llvm-lib and llvm-dlltool driver logic |
| `Transforms/` | IR transformation passes (InstCombine, vectorizer, scalar opts) |
| `WindowsDriver/` | Windows-specific driver utilities |
| `WindowsManifest/` | Windows manifest file embedding |
| `XRay/` | XRay instrumentation runtime support |

## For AI Agents

### Working In This Directory
- Each subdirectory builds into its own CMake target (e.g., `LLVMCodeGen`, `LLVMSupport`)
- Add new `.cpp` files to the subdirectory's `CMakeLists.txt` `add_llvm_component_library()` call
- Generated `.inc` files come from TableGen — do not edit them directly

### Common Patterns
- Implementation files `#include` their own component's headers plus `llvm/Support/` utilities
- Pass implementations register themselves via static initializers using `INITIALIZE_PASS` macros

<!-- MANUAL: -->
