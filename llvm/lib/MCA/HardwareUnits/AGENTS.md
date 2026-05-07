<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/MCA/HardwareUnits

## Purpose

Simulated hardware components for the MCA pipeline. Models execution units, register files, memory subsystems, and retirement logic. Each unit enforces hardware constraints (latencies, port availability, register pressure) during instruction simulation.

## Key Files

| File | Category | Description |
|------|----------|-------------|
| `HardwareUnit.cpp` | Base | Abstract hardware unit interface |
| `Scheduler.cpp` | Execution | Instruction scheduler (ready queue, resource reservation, latency tracking) |
| `RegisterFile.cpp` | Registers | Register file simulation (reads, writes, availability) |
| `LSUnit.cpp` | Memory | Load/Store unit (memory ordering, coherency, cache modeling) |
| `RetireControlUnit.cpp` | Retirement | Retirement logic (instruction finalization, result writeout) |
| `ResourceManager.cpp` | Resources | Resource pool management (execution ports, functional units) |

## For AI Agents

### Working In This Directory

These files model CPU hardware constraints that the pipeline must respect. When working here:

1. **HardwareUnit abstraction**: All units inherit from HardwareUnit. Implement isReady() and execute() or issue() appropriately.
2. **Scheduler logic**: The Scheduler maintains instruction dependencies and issues instructions to available execution ports. Understand instruction readiness vs. port availability.
3. **Register pressure**: RegisterFile tracks register usage. Instructions block if destination register not available.
4. **Memory modeling**: LSUnit enforces memory ordering and models load/store latencies. Handle false dependencies and memory barriers.
5. **Retirement**: RetireControlUnit finishes instructions, writes results, commits state. Order matters for exception handling.
6. **Resource tracking**: ResourceManager allocates execution ports. Each instruction uses specific ports for different operation types.

### Common Patterns

- **Unit initialization**: Query MCSchedModel for latencies, resource counts, execution ports from target.
- **Instruction readiness**: Check operand availability (registers), dependencies (prior instructions), and resource availability.
- **Latency tracking**: Model multi-cycle operations (multiplies, divisions, memory). Track in-flight cycles.
- **Port allocation**: Assign instructions to specific execution ports based on operation type and port availability.
- **Register locking**: Block instructions until destination registers available. Handle register renaming conceptually.
- **Memory coherency**: Track loads/stores in order. Handle memory dependencies and barriers.

## Dependencies

### Internal
- `llvm/MC/MCInst.h` — Instruction representation
- `llvm/MC/MCInstrInfo.h` — Instruction properties (latencies, resource usage)
- `llvm/MC/MCSchedule.h` — CPU scheduling model
- `llvm/MC/MCSubtargetInfo.h` — CPU feature sets
- `llvm/Support/` — Utilities
- `llvm/MCA/Instruction.h` — Simulated instruction representation
- `llvm/MCA/Context.h` — MCA execution context

### External
- Standard C++ library

<!-- MANUAL: -->
