<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/MCA/Stages

## Purpose

Pipeline stages — the execution phases of the MCA simulation engine. Each stage represents a processing step (fetch, dispatch, issue, execute, retire). Stages process instructions in sequence, enforcing timing and resource constraints.

## Key Files

| File | Description |
|------|-------------|
| `Stage.cpp` | Abstract stage base class (interface, cycle execution) |
| `EntryStage.cpp` | Entry stage (load instructions from trace into pipeline) |
| `DispatchStage.cpp` | Dispatch stage (rename registers, allocate resources, emit ready instructions) |
| `InOrderIssueStage.cpp` | In-order issue stage (issue instructions to execution ports in sequence) |
| `MicroOpQueueStage.cpp` | Micro-op queue (buffer for micro-ops before dispatch) |
| `ExecuteStage.cpp` | Execute stage (simulate instruction execution on hardware units) |
| `RetireStage.cpp` | Retire stage (finalize instructions, write results, update architectural state) |
| `InstructionTables.cpp` | Instruction property tables (latencies, resources) |

## For AI Agents

### Working In This Directory

Pipeline stages are the core simulation units. Each stage processes in-flight instructions every cycle. When working here:

1. **Stage ordering**: Stages execute in pipeline order (Entry → Dispatch → Issue → Execute → Retire). Each stage has its own queue.
2. **Cycle-by-cycle execution**: Each stage processes instructions every cycle. Blocking one stage can stall earlier stages.
3. **In-order vs out-of-order**: InOrderIssueStage enforces program order. Other designs (like modern CPUs) use out-of-order issue.
4. **Stage transitions**: Instructions move between stages (Entry → Dispatch → Issue → Execute → Retire). Blocking at one stage stalls prior stages.
5. **Resource contention**: ExecuteStage requests execution ports from ResourceManager. Block if unavailable.
6. **Dependencies**: Track RAW, WAR, WAW dependencies. Instruction can't issue until dependencies resolved.

### Common Patterns

- **Stage lifecycle**: Execute once per cycle. Process all in-flight instructions. Return true if progress made.
- **Queue management**: Each stage has input/output queues. Move ready instructions downstream.
- **Blocking logic**: Return false from execute() if the stage can't make progress (stalled). This blocks prior stages.
- **Resource queries**: Ask ResourceManager if instruction can execute on available ports.
- **Instruction advancement**: Move instructions from input queue to output queue. Update instruction state.
- **Cycle tracking**: Increment cycle counter. Track in-flight latencies per instruction.

## Stage Execution Model

```
EntryStage: Loads instructions from trace
    ↓
DispatchStage: Allocates resources, performs renaming
    ↓
InOrderIssueStage: Checks dependencies, issues to execution ports
    ↓
ExecuteStage: Simulates execution on hardware units
    ↓
RetireStage: Commits results, finalizes instructions
```

Each stage runs every cycle and processes its queue. If a stage blocks (returns false), the prior stages are also blocked in the next cycle.

## Dependencies

### Internal
- `llvm/MC/MCInstr.h` — Instruction representation
- `llvm/MC/MCInstrInfo.h` — Instruction properties
- `llvm/MC/MCSchedule.h` — CPU scheduling model
- `llvm/Support/` — Utilities
- `llvm/MCA/Instruction.h` — Simulated instruction
- `llvm/MCA/Context.h` — MCA execution context
- `llvm/MCA/HardwareUnits/` — Execution units, scheduler, register file

### External
- Standard C++ library

<!-- MANUAL: -->
