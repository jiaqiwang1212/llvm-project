<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# WebAssembly Backend

## Purpose
Targets WebAssembly (wasm32/wasm64), a portable binary instruction format for the web and edge computing. Used for browser execution, serverless functions, and embedded systems.

## Key Files
| File | Description |
|------|-------------|
| `WebAssemblyTargetMachine.cpp` | TargetMachine implementation |
| `WebAssemblyISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `WebAssemblyISelLowering.cpp` | Calling convention and custom DAG lowering |
| `WebAssemblyAsmPrinter.cpp` | WebAssembly text/binary output |
| `WebAssemblyInstrInfo.cpp` | Instruction info implementation |
| `WebAssemblyRegisterInfo.cpp` | Register info implementation |
| `WebAssemblyFrameLowering.cpp` | Stack frame handling |
| `WebAssembly.td` | Main target definition |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |
| `GISel/` | Global instruction selection |
| `MCTargetDesc/` | MC target description |
| `TargetInfo/` | Target information |
| `Utils/` | Utility code |

## For AI Agents

### Working In This Directory
- Edit WebAssembly.td for instruction changes, then rebuild with llvm-tblgen
- WebAssemblyISelLowering.cpp handles calling conventions and custom DAG lowering
- WebAssemblyAsmPrinter.cpp generates wasm text/binary format
- WebAssemblyCFGStackify.cpp performs stack machine transformation
- WebAssemblyArgumentMove.cpp handles argument passing

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
