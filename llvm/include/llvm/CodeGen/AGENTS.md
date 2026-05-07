<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CodeGen

## Purpose

Target-independent code generation infrastructure. This directory contains the core abstractions for converting LLVM IR into machine code: MachineFunction and basic block graphs, machine instructions and operands, instruction selection (SelectionDAG, FastISel), instruction lowering, register allocation, scheduling, and machine function optimization passes.

## Key Files

| File | Purpose |
|------|---------|
| `MachineFunction.h` | Container for target code, basic blocks, and metadata for a function |
| `MachineBasicBlock.h` | Sequence of machine instructions, target-level equivalent of BasicBlock |
| `MachineInstr.h` | Target instruction representation |
| `MachineOperand.h` | Operand of a machine instruction (register, immediate, memory, jump table, etc.) |
| `TargetLowering.h` | Abstract interface for target-specific IR lowering decisions |
| `SelectionDAG.h` | Directed acyclic graph for instruction selection |
| `SelectionDAGISel.h` | Instruction selection from SelectionDAG to machine instructions |
| `FastISel.h` | Fast instruction selection without SelectionDAG (for debug builds, quick codegen) |
| `CallingConvLower.h` | Helper to lower function call arguments and return values per target ABI |
| `TargetRegisterInfo.h` | Abstract interface: register names, classes, reserved registers, calling conventions |
| `TargetFrameLowering.h` | Abstract interface: function prologue/epilogue, stack frame layout |
| `TargetInstrInfo.h` | Abstract interface: instruction properties, copy/move, branch analysis |
| `TargetSubtargetInfo.h` | Abstract interface: CPU features, instruction scheduling, late refinements |
| `RegAllocBase.h` | Base class for register allocators |
| `RegAllocFast.h` | Fast register allocator (linear scan, no optimization) |
| `RegAllocGreedyPass.h` | Greedy register allocator (production quality) |
| `LiveInterval.h` | Live range of a virtual register |
| `LiveVariables.h` | Which variables are live at each instruction |
| `LiveRangeEdit.h` | Edits to live ranges during register allocation |
| `SlotIndexes.h` | Maps instructions to index numbers for live interval queries |
| `ScheduleDAG.h` | Directed acyclic graph for instruction scheduling |
| `ScheduleDAGInstrs.h` | ScheduleDAG built from machine instructions and data dependencies |
| `MachineScheduler.h` | Machine scheduling passes |
| `PrologEpilogInserter.h` | Inserts prologue and epilogue code |
| `TargetPassConfig.h` | Framework for per-target pass pipeline configuration |
| `Passes.h` | Declarations of all standard code generation passes |
| `AsmPrinter.h` | Base class for target assembly language printers |

## For AI Agents

### Working In This Directory

**Understand the MachineFunction hierarchy first:**
- MachineFunction is the top-level container for target code
- MachineBasicBlock holds a sequence of machine instructions
- MachineInstr represents a single instruction with operands
- MachineOperand represents register, immediate, memory, or jump table references

**Instruction selection flow:**
1. SelectionDAG (Directed Acyclic Graph) is built from LLVM IR and target-specific operations
2. SelectionDAGISel walks the DAG and emits MachineInstrs
3. FastISel provides a quicker path without DAG construction

**Register allocation:**
- LiveInterval tracks the live range of a virtual register
- Register allocators assign virtual registers to physical registers
- RegAllocBase is the interface; RegAllocFast and RegAllocGreedyPass are implementations

**Target abstraction layers:**
- `TargetLowering` - decisions about IR lowering (how to lower LLVM IR operations)
- `TargetRegisterInfo` - register properties, calling conventions
- `TargetFrameLowering` - prologue/epilogue, stack layout
- `TargetInstrInfo` - instruction properties and analysis
- `TargetSubtargetInfo` - CPU features and scheduling

### Common Patterns

**Query/modify MachineFunction:**
```cpp
MachineFunction &MF = ...;
for (MachineBasicBlock &MBB : MF) {
  for (MachineInstr &MI : MBB) {
    for (MachineOperand &MO : MI.operands()) {
      // Analyze or transform operands
    }
  }
}
```

**Check target capabilities:**
```cpp
const TargetLowering &TLI = MF.getTarget().getTargetLowering();
if (TLI.isTypeLegal(VT)) { ... }
```

**Access register info:**
```cpp
const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
for (unsigned Reg : TRI->registers()) { ... }
```

**Schedule inspection:**
```cpp
ScheduleDAGInstrs DAG(MI, false);
DAG.buildSchedGraph(nullptr);
```

## Dependencies

### Internal
- `IR/` - LLVM types, IR instructions, functions (lowered to MachineFunction)
- `Analysis/` - dominators, loops, callgraph, branch probabilities
- `Transforms/` - passes that may run before CodeGen (GlobalISel, early lowering)
- `Support/` - data structures, error handling, pass infrastructure

### External (other CodeGen directories)
- `Target/` - TargetMachine, TargetRegistry
- `MC/` - MCInst, MCStreamer (after instruction emission)

<!-- MANUAL: -->
