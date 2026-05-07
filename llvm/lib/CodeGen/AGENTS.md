<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CodeGen

## Purpose

Target-independent code generation pipeline. Transforms LLVM IR into machine-dependent code (MIR/assembly). Implements instruction selection, register allocation, scheduling, and code emission. All target-independent compilation phases happen here.

## Key Files

| File | Purpose |
|------|---------|
| `MachineInstr.cpp` | MachineInstruction representation, flags, operand management |
| `MachineBasicBlock.cpp` | MachineBasicBlock container for machine instructions |
| `MachineFunction.cpp` | MachineFunction IR representation, function properties |
| `RegAllocGreedy.cpp` | Greedy register allocator (primary LLVM allocator) |
| `RegAllocFast.cpp` | Fast register allocator (O(n) for fast compilation modes) |
| `MachineFunctionPass.cpp` | Base class for machine IR passes |
| `PrologEpilogInserter.cpp` | Prolog/epilog insertion, stack frame setup |
| `TwoAddressInstructionPass.cpp` | Converts 2-address machine instructions to 3-address |
| `PHIElimination.cpp` | Converts PHI nodes to machine code copies |
| `BranchFolding.cpp` | Tail merging and branch folding optimization |
| `IfConversion.cpp` | If-conversion (branch elimination via predication) |
| `PostRASchedulerList.cpp` | Post-register-allocation instruction scheduler |
| `MachineScheduler.cpp` | Generic machine instruction scheduler framework |
| `LiveInterval.cpp` | Live interval representation and computation |
| `LiveIntervals.cpp` | Liveness analysis for register allocation |
| `LiveRangeEdit.cpp` | Live range editing (splitting, utilities) |
| `SpillPlacement.cpp` | Heuristics for optimal spill code placement |
| `RegisterCoalescer.cpp` | Copy coalescing to reduce register pressure |
| `VirtRegMap.cpp` | Virtual-to-physical register mapping |
| `MachineVerifier.cpp` | Verification pass for machine IR integrity |
| `TargetPassConfig.cpp` | Target-specific pass pipeline configuration |
| `CodeGen.cpp` | Top-level CodeGen initialization |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `AsmPrinter/` | Assembly/object file emission, DWARF debug info (see `AsmPrinter/AGENTS.md`) |
| `GlobalISel/` | Global Instruction Selection (ISel 2.0, generic MIR-based) (see `GlobalISel/AGENTS.md`) |
| `LiveDebugValues/` | Variable location debug info propagation (see `LiveDebugValues/AGENTS.md`) |
| `MIRParser/` | Machine IR text format parser for testing (see `MIRParser/AGENTS.md`) |
| `SelectionDAG/` | SelectionDAG instruction selection pipeline (see `SelectionDAG/AGENTS.md`) |

## For AI Agents

### Working In This Directory

The CodeGen directory contains the core machine IR pipeline. Start here for:
- Machine instruction semantics and representation
- Register allocation algorithms
- Instruction scheduling
- Machine IR passes (MachineFunctionPass subclasses)
- Machine liveness analysis
- Frame info and prologue/epilogue

Key pattern: Most work is `MachineFunctionPass` subclasses that iterate over machine functions and basic blocks. Understand `MachineFunction`, `MachineBasicBlock`, and `MachineInstr` first.

### Common Patterns

**Machine Passes**: Every optimization is typically a `MachineFunctionPass` that overrides `runOnMachineFunction()`:
```cpp
class MyPass : public MachineFunctionPass {
  bool runOnMachineFunction(MachineFunction &MF) override {
    for (auto &MBB : MF)
      for (auto &MI : MBB) {
        // Work on MI
      }
    return Changed;
  }
};
```

**Liveness Analysis**: Register allocation and scheduling use `LiveIntervals` (computed by `LiveIntervalAnalysis`). These are queried via `MachineRegisterInfo` and `VirtRegMap`.

**Target Hooks**: Machine passes often call target-specific info via `TargetInstrInfo`, `TargetRegisterInfo`, `TargetFrameLowering`, etc. These are provided by each backend.

**MachineOperand**: Instructions contain operands (registers, immediates, globals, etc.). Use `MachineOperand::getType()` to distinguish register vs. memory vs. immediate.

## Dependencies

### Internal
- `include/llvm/CodeGen/` — Machine IR headers (MachineInstr, MachineFunction, LiveIntervals, etc.)
- `include/llvm/Target/TargetMachine.h` — Base target machine class
- `AsmPrinter/` — Assembly emission (depends on CodeGen output)
- `SelectionDAG/` — SelectionDAG ISel (depends on CodeGen for MachineFunction)
- `GlobalISel/` — GlobalISel ISel alternative (depends on CodeGen for MachineFunction)
- `LiveDebugValues/` — Debug value propagation (operates on machine IR post-ISel)
- `MIRParser/` — Parses .mir text (testing/debugging tool)

### External
- `IR/` — LLVM IR types and Values (pre-CodeGen)
- `Analysis/` — CFG, dominance, loop info, etc. (available at machine IR level)
- Each target's `Target/*/` directory for target-specific implementations

<!-- MANUAL: -->
