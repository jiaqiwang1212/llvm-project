<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/MCA

## Purpose

Machine Code Analyzer (MCA) — a simulation engine that analyzes the performance of machine code on a specific CPU model. MCA executes instructions through a configurable pipeline, models hardware constraints (execution units, registers, memory), and reports performance metrics (latency, throughput, resource usage). Used by llvm-mca tool and performance debugging.

## Key Files

| File | Category | Description |
|------|----------|-------------|
| `Context.cpp` | Core | MCA execution context (state, statistics, callbacks) |
| `Pipeline.cpp` | Pipeline | Instruction pipeline orchestrator (coordinates stages) |
| `CodeEmitter.cpp` | Core | Code loading and trace preparation |
| `InstrBuilder.cpp` | Core | MCInst to Instruction conversion (enriches with latency info) |
| `Instruction.cpp` | Core | Simulated instruction (opcode, operands, latencies, dependencies) |
| `CustomBehaviour.cpp` | Extensions | Custom CPU behavior hooks for model-specific quirks |
| `Support.cpp` | Utilities | MCA support utilities |
| `View.cpp` | Reporting | Abstract view interface for results |
| `HWEventListener.cpp` | Instrumentation | Hardware event listener (callback interface) |
| `IncrementalSourceMgr.cpp` | Source | Source code manager for tracing |
| `HardwareUnits/` | Hardware | Simulated hardware units (execution, retirement, register file, load/store) |
| `Stages/` | Pipeline | Pipeline stages (fetch, dispatch, execute, retire) |
| `CMakeLists.txt` | Build | MCA build configuration |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `HardwareUnits/` | Simulated hardware components (Scheduler, RegisterFile, LSUnit, RetireControlUnit, ResourceManager) — see `HardwareUnits/AGENTS.md` |
| `Stages/` | Pipeline stages (EntryStage, DispatchStage, ExecuteStage, RetireStage) — see `Stages/AGENTS.md` |

## For AI Agents

### Working In This Directory

MCA simulates machine code execution on a modeled CPU. When working here:

1. **Simulation model**: MCA models a specific CPU (instruction latencies, execution units, memory hierarchy). Check target machine specs.
2. **Pipeline stages**: Instructions flow through stages (fetch → dispatch → execute → retire). Each stage enforces constraints.
3. **Dependency tracking**: Track instruction dependencies (data, resource, memory). The scheduler respects these.
4. **Hardware units**: Execution ports, registers, memory units are modeled. Resource contention causes pipeline stalls.
5. **Extensibility**: CustomBehaviour allows target-specific overrides. Use callbacks for non-standard behavior.
6. **Statistics**: Collect cycle counts, instruction latencies, resource utilization. Report via View abstraction.

### Common Patterns

- **Instruction simulation**: Load with CodeEmitter, convert with InstrBuilder, execute through pipeline.
- **Stage coordination**: Pipeline orchestrates stages. Each stage processes in-flight instructions.
- **Dependency resolution**: Check register dependencies before dispatch, memory dependencies before execute.
- **Resource management**: Query ResourceManager for available execution units. Block on unavailability.
- **View callbacks**: Use View to print reports (instruction latency, resource usage, warnings).

## Dependencies

### Internal
- `llvm/MC/MCInst.h` — Instruction representation
- `llvm/MC/MCInstrInfo.h` — Instruction properties
- `llvm/MC/MCRegisterInfo.h` — Register information
- `llvm/MC/MCSubtargetInfo.h` — CPU feature sets
- `llvm/Support/` — Utilities, error handling
- Target backends (llvm/Target/) — MCInstrInfo, MCRegisterInfo, subtarget info, MCSchedule

### External
- Standard C++ library

<!-- MANUAL: -->
