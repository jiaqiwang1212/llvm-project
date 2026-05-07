<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# llvm (C++ public headers)

## Purpose
The primary C++ public API for the LLVM library. Each subdirectory corresponds to a logical component. Code that uses LLVM as a library includes from here (`#include "llvm/<Component>/..."`).

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `ABI/` | Platform-specific ABI classification (argument passing, return types) |
| `ADT/` | Abstract data types (SmallVector, DenseMap, StringRef, etc.) |
| `Analysis/` | IR analysis pass interfaces (alias analysis, loop info, dominator tree) |
| `AsmParser/` | LLVM IR textual assembly parser interface |
| `BinaryFormat/` | Binary format constants and structs (ELF, COFF, MachO, Wasm, DWARF) |
| `Bitcode/` | Bitcode reader/writer interface |
| `Bitstream/` | Low-level bitstream container reader interface |
| `CAS/` | Content-addressable storage interface |
| `CGData/` | Code generation data collection interface |
| `CodeGen/` | Target-independent code generation interfaces (ISel, RA, MachineIR) |
| `CodeGenTypes/` | Type lowering from IR types to machine types |
| `Config/` | Build-system-generated configuration (`llvm-config.h`) |
| `DebugInfo/` | Debug information interfaces (DWARF, CodeView, GSYM, PDB) |
| `Debuginfod/` | Debuginfod HTTP client interface |
| `Demangle/` | Symbol demangling interface |
| `DTLTO/` | Distributed ThinLTO interface |
| `DWARFCFIChecker/` | DWARF CFI consistency checker |
| `DWARFLinker/` | DWARF linker interface |
| `DWP/` | DWARF package writer interface |
| `ExecutionEngine/` | JIT execution engine interfaces (ORC, MCJIT) |
| `FileCheck/` | FileCheck pattern-matching interface |
| `Frontend/` | Frontend support (OpenMP, OpenACC, HLSL, Offloading, Directives) |
| `FuzzMutate/` | IR mutation for fuzzing |
| `HTTP/` | HTTP client interface |
| `InterfaceStub/` | ELF interface stub generator |
| `IR/` | Core LLVM IR types (Module, Function, Instruction, Type, Value, etc.) |
| `IRPrinter/` | IR pretty-printing pass interface |
| `IRReader/` | Unified IR reader (bitcode + textual) |
| `LineEditor/` | Interactive line editor with history |
| `Linker/` | IR-level linker interface |
| `LTO/` | Link-time optimization interface |
| `MC/` | Machine code layer (assembler, disassembler, streamer, fixups) |
| `MCA/` | Machine code analyzer interface |
| `ObjCopy/` | Object file transformation interface |
| `Object/` | Object file reading interface |
| `ObjectYAML/` | YAML ↔ object file interface |
| `Option/` | Command-line option parsing interface |
| `Passes/` | Pass manager and pipeline builder interfaces |
| `Plugins/` | Dynamic pass plugin interface |
| `ProfileData/` | Profile data reading/writing interface |
| `Remarks/` | Optimization remark serialization interface |
| `SandboxIR/` | Sandboxed IR for safe transformations |
| `Support/` | Core utilities (error handling, file system, memory, threads, strings) |
| `TableGen/` | TableGen language interface |
| `Target/` | Target description interfaces (TargetMachine, TargetInfo, etc.) |
| `TargetParser/` | Target triple and CPU feature string parsing |
| `Telemetry/` | Telemetry framework interface |
| `Testing/` | Test infrastructure helpers |
| `TextAPI/` | Apple TBD/TextAPI interface |
| `ToolDrivers/` | llvm-lib / llvm-dlltool driver interface |
| `Transforms/` | IR transformation pass interfaces |
| `WindowsDriver/` | Windows driver utilities interface |
| `WindowsManifest/` | Windows manifest embedding |
| `WindowsResource/` | Windows resource file parsing |
| `XRay/` | XRay instrumentation interface |

## For AI Agents

### Working In This Directory
- All headers use `#pragma once`
- Forward declarations of major types belong in the component's own `Forward.h` or included via `llvm/IR/Value.h` etc.
- Avoid including heavy headers in other headers — use forward declarations to keep compile times reasonable
- `LLVM_ABI` marks symbols that should be exported from a shared library build

### Common Patterns
- Pass interfaces live in `Passes/` (new pass manager) or as `FunctionPass`/`ModulePass` subclasses (legacy)
- `Analysis/` headers declare the `AnalysisKey` and `Result` types for new-PM analyses
- TableGen-generated headers (`.inc`) are generated into the build directory and included with `#include`

<!-- MANUAL: -->
