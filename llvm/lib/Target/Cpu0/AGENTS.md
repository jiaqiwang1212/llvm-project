<!-- Generated: 2026-04-23 | Updated: 2026-05-07 -->

# Cpu0 Target Backend

## Purpose
LLVM backend for the Cpu0 tutorial architecture — a 32-bit RISC ISA modeled after MIPS, used in the "Tutorial: Creating an LLVM Backend for the Cpu0 Architecture" series. Implements the full code-generation pipeline from DAG lowering through machine code emission, built chapter-by-chapter via `Cpu0SetChapter.h`.

## Key Files

| File | Description |
|------|-------------|
| `Cpu0.h` | Top-level entry point; declares `createCpu0ISelDag` pass factory and `initializeCpu0DAGToDAGISelLegacyPass` |
| `Cpu0SetChapter.h` | Chapter gate — `#define CH CHx_y` controls which code sections are compiled, enabling incremental tutorial builds |
| `Cpu0Config.h` | Compile-time feature flags |
| `Cpu0TargetMachine.h/.cpp` | Owns `Cpu0Subtarget`, `Cpu0ABIInfo`, and `TLOF`; registers big/little-endian variants (`Cpu0ebTargetMachine`, `Cpu0elTargetMachine`); data layout is `e/E-m:m-p:32:32-i8:8:32-i16:16:32-i64:64-n32-S64` |
| `Cpu0Subtarget.h/.cpp` | Aggregates `RegInfo`, `InstrInfo`, `FrameLowering`, `TLInfo`; holds arch version (`Cpu032I`/`Cpu032II`), endianness, and feature flags like `HasCmp`, `HasSlt` |
| `Cpu0ISelDAGToDAG.h/.cpp` | DAG-to-DAG instruction selector; implements `Select()` and `SelectAddr()` for base+offset addressing |
| `Cpu0SEISelDAGToDAG.h/.cpp` | Standard-edition DAG-to-DAG selector specialization |
| `Cpu0ISelLowering.h/.cpp` | DAG lowering; defines `Cpu0ISD` opcodes (`Ret`, `JmpLink`, `Hi`, `Lo`, `GPRel`, `DivRem`, etc.); handles formal arguments and returns |
| `Cpu0SEISelLowering.h/.cpp` | Standard-edition specialization of `Cpu0TargetLowering` |
| `Cpu0InstrInfo.h/.cpp` | Instruction info base class |
| `Cpu0SEInstrInfo.h/.cpp` | Standard-edition instruction info; factory `createCpu0SEInstrInfo` |
| `Cpu0InstrInfo.td` | TableGen instruction definitions — primary source for most ISel patterns |
| `Cpu0InstrFormats.td` | Encoding format classes for all instruction types |
| `Cpu0CallingConv.td` | Calling convention definitions consumed by ISel lowering |
| `Cpu0RegisterInfo.h/.cpp` | Register info base class |
| `Cpu0SERegisterInfo.h/.cpp` | Standard-edition register info |
| `Cpu0RegisterInfo.td` | Register and register-class definitions |
| `Cpu0RegisterInfoGPROutForOther.td` | Alternate GPR allocation order variant |
| `Cpu0Schedule.td` | Scheduling itineraries |
| `Cpu0Other.td` | Miscellaneous TableGen definitions |
| `Cpu0.td` | Root TableGen file that includes all `.td` fragments |
| `Cpu0FrameLowering.h/.cpp` | Abstract frame lowering base; stack grows down; factory `create(STI)` |
| `Cpu0SEFrameLowering.h/.cpp` | Standard-edition frame lowering; emits prologue/epilogue |
| `Cpu0MachineFunction.h/.cpp` | Per-function state (`Cpu0FunctionInfo`) |
| `Cpu0AsmPrinter.h/.cpp` | MC-level assembly printer |
| `Cpu0MCInstLower.h/.cpp` | Lowers `MachineInstr` to `MCInst` for emission |
| `Cpu0TargetObjectFile.h/.cpp` | ELF object-file lowering |
| `Cpu0TargetStreamer.h` | Target-specific assembly streamer for directives |
| `Cpu0AnalyzeImmediate.h/.cpp` | Analyzes and decomposes large immediates into instruction sequences (ADDiu, ORi, SHL, LUi) |
| `CMakeLists.txt` | Build rules for the root target library |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `InstPrinter/` | Converts MCInst to human-readable assembly text (see `InstPrinter/AGENTS.md`) |
| `MCTargetDesc/` | MC-layer target description: register/instruction enums, ABI info, assembler info (see `MCTargetDesc/AGENTS.md`) |
| `TargetInfo/` | LLVM target registry — exposes `getTheCpu0Target()` / `getTheCpu0elTarget()` (see `TargetInfo/AGENTS.md`) |

## For AI Agents

### Working In This Directory

- **Chapter gating**: `Cpu0SetChapter.h` defines `CH` to a chapter token (e.g. `CH3_5`). Code blocks guarded by `#if CH >= CHx_y` only compile at or after that chapter. When adding new functionality, choose the correct chapter guard or leave unguarded if it applies from the start.
- **SE vs base classes**: Many classes come in a base + `SE` (Standard Edition) pair (`Cpu0FrameLowering` / `Cpu0SEFrameLowering`, etc.). The base holds the abstract interface; `SE` contains the concrete implementation. New methods belong in `SE` unless they are truly architecture-invariant.
- **TableGen pipeline**: `.td` files generate `Cpu0Gen*.inc` files at build time. Do not edit `.inc` files directly. After changing `.td` files, rebuild to regenerate includes.
- **Endianness**: `Cpu0ebTargetMachine` (big) and `Cpu0elTargetMachine` (little) share a single `Cpu0TargetMachine` base. The `IsLittle` flag flows through `computeDataLayout` and into `Subtarget`.

### Testing Requirements

- Build via `cmake --build build --target Cpu0` (or equivalent ninja target) and check that `llc -march=cpu0` can process a `.bc` file without crashing.
- The tutorial uses `ch3.bc` through `chN.bc` test inputs corresponding to chapter milestones.

### Common Patterns

- Pass factories return `FunctionPass*` and are registered with `initializeCpu0DAGToDAGISelLegacyPass`.
- `Cpu0Subtarget` owns all the hook objects (`InstrInfo`, `FrameLowering`, `TLInfo`) as `std::unique_ptr` members; retrieve them through `getInstrInfo()`, `getFrameLowering()`, `getTargetLowering()`.
- `Cpu0ISD::NodeType` enums start at `ISD::BUILTIN_OP_END` to avoid collisions with generic LLVM opcodes.

## Dependencies

### Internal
- `MCTargetDesc/` — register/instruction enums via `Cpu0GenRegisterInfo.inc`, `Cpu0GenInstrInfo.inc`
- `TargetInfo/` — target object returned by `getTheCpu0Target()`
- `InstPrinter/` — used by `Cpu0AsmPrinter` for text emission

### External
- `llvm/CodeGen/` — `SelectionDAGISel`, `TargetLowering`, `TargetFrameLowering`, `TargetPassConfig`
- `llvm/MC/` — `MCInstPrinter`, `TargetRegistry`
- `llvm/Target/` — `TargetMachine`

<!-- MANUAL: -->
