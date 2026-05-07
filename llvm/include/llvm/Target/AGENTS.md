<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Target

## Purpose

Target machine abstraction and registry. Defines the interfaces that all backends implement: TargetMachine (entry point), TargetOptions (configuration), TargetInfo (target-specific info), TargetRegistry (discovers available targets), and target description files (`.td` files) that declaratively describe instructions, registers, and calling conventions.

## Key Files

| File | Purpose |
|------|---------|
| `TargetMachine.h` | Abstract interface: entry point for a backend, owns passes, options, and lowering logic |
| `TargetOptions.h` | Configuration options that affect code generation (optimization level, debug info, etc.) |
| `TargetLoweringObjectFile.h` | Target-specific object file format handling (sections, relocations) |
| `TargetRegistry.h` | Global registry of available targets; macros to self-register backends |
| `CGPassBuilderOption.h` | Options for constructing the CodeGen pass pipeline |
| `RegisterTargetPassConfigCallback.h` | Callback mechanism for targets to customize pass configuration |
| `CodeGenCWrappers.h` | C API wrappers for target operations |
| `Target.td` | Target description root file (templates and base definitions) |
| `TargetCallingConv.td` | Declarative calling convention definitions (registers, stack layout) |
| `TargetItinerary.td` | Instruction itinerary (pipeline model, instruction latencies) |
| `TargetSchedule.td` | Scheduler model definitions |
| `TargetSelectionDAG.td` | Patterns for lowering IR operations to target instructions |
| `TargetInstrPredicate.td` | Instruction predicate definitions |
| `TargetMacroFusion.td` | Macro fusion rules (fusing adjacent instructions) |
| `TargetPfmCounters.td` | Performance counter definitions for profiling |
| `GenericOpcodes.td` | Generic opcodes for GlobalISel |

## For AI Agents

### Working In This Directory

**Understand target registration:**
- Targets self-register via `LLVM_TARGET(Arch, TargetClass)` macro in TargetRegistry.h
- TargetMachine is the factory and entry point for a backend
- Each backend creates a subclass of TargetMachine and registers it

**TargetMachine responsibilities:**
- Owns TargetOptions and configuration
- Creates the pass pipeline (via TargetPassConfig subclass)
- Provides access to TargetLowering, TargetRegisterInfo, TargetInstrInfo, etc.
- Implements code generation entry points

**Target description files (`.td`):**
- Declarative specifications processed by TableGen to generate C++ code
- Define instructions, registers, register classes, calling conventions
- Not executed directly; TableGen processes them to generate `-gen-*.inc` files
- Key patterns: `class`, `def`, `foreach`, `!cond()`, pattern matching

**Target interfaces (implemented by each backend):**
- `TargetLowering` - IR lowering decisions
- `TargetRegisterInfo` - registers and calling conventions
- `TargetFrameLowering` - prologue/epilogue, stack
- `TargetInstrInfo` - instruction properties, codegen patterns
- `TargetSubtargetInfo` - CPU features, scheduling

### Common Patterns

**Access TargetMachine:**
```cpp
const TargetMachine &TM = MF.getTarget();
const TargetLowering &TLI = TM.getTargetLowering();
const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
```

**Register a target (in backend TargetInfo.cpp):**
```cpp
extern "C" void LLVMInitializeMyTargetInfo() {
  RegisterTargetMachine<MyTargetMachine> X(getTheMyTarget());
}
```

**TableGen instruction pattern (from `.td`):**
```
def ADD : Instruction {
  let Opcode = 0x01;
  let OutOperandList = (outs i32:$dest);
  let InOperandList = (ins i32:$lhs, i32:$rhs);
}

def : Pat<(add i32:$x, i32:$y), (ADD $x, $y)>;
```

## Dependencies

### Internal
- `CodeGen/` - TargetLowering, TargetRegisterInfo, etc. are subclassed here
- `MC/` - MCInstrInfo, MCRegisterInfo, MCSubtargetInfo (machine code layer)
- `Support/` - pass infrastructure, error handling
- `IR/` - LLVM types and operations

### External (backend-specific)
- Each backend (X86, ARM, RISCV, etc.) implements these interfaces

<!-- MANUAL: -->
