<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MCA

## Purpose

Machine Code Analyzer: simulates instruction execution on a target CPU pipeline to predict throughput, latency, and resource conflicts. Used by the `llvm-mca` tool to analyze instruction sequences without running them on real hardware. Provides pipeline modeling, scheduler simulation, and performance metrics.

## Key Files

| File | Purpose |
|------|---------|
| `Pipeline.h` | Main pipeline simulator; orchestrates stages and hardware units |
| `InstrBuilder.h` | Converts MCInst to Instruction (adds latencies, resource info) |
| `Instruction.h` | Represents an instruction in the pipeline with operand dependencies |
| `Stages/` | Pipeline stages (Fetch, Decode, Execute, Retire, etc.) |
| `HardwareUnits/` | Simulated hardware: RegisterFile, LoadStoreUnit, ALU, etc. |
| `Context.h` | Simulation context (target info, instruction metadata) |
| `SourceMgr.h` | Manages input instruction stream |
| `HWEventListener.h` | Observer interface for pipeline events |
| `IncrementalSourceMgr.h` | Incremental instruction fetching |
| `CustomBehaviour.h` | Hook for target-specific pipeline behavior |
| `View.h` | Abstract interface for statistics views (output reports) |
| `Support.h` | Utility functions for analysis |
| `CodeEmitter.h` | Emits instructions for analysis |

## For AI Agents

### Working In This Directory

**Understand the MCA pipeline:**
- MCInst (from MC/) is decoded into Instruction (adds semantic info: latencies, dependencies, resources)
- InstrBuilder performs this conversion using MCInstrInfo and scheduling models
- Pipeline simulates execution: Fetch → Decode → Execute → Retire stages
- HardwareUnits model resources: RegisterFile, Load/StoreUnit, execution units (ALU, FPU, etc.)
- Events fire on instruction transitions (e.g., instruction ready, instruction retired)
- Views aggregate statistics (throughput, latency, resource pressure, etc.)

**Key concepts:**
- **Latency**: cycles before result is available
- **Throughput**: how many instructions execute per cycle (IPC)
- **Resource pressure**: how much each hardware unit is utilized
- **Dependencies**: data hazards (register read-after-write, memory ordering)

**Typical analysis flow:**
1. Read MCInst sequence (from assembly or inline code)
2. InstrBuilder converts to Instruction with timing info
3. Pipeline simulator executes the sequence
4. Stages advance instructions through pipeline
5. HardwareUnits track resource availability
6. Views produce reports (latency, throughput, resource utilization)

### Common Patterns

**Set up a simple simulation:**
```cpp
Context &C = ...;  // Target context (from MCSubtargetInfo)
InstrBuilder IB(C);

std::vector<MCInst> Insts = ...;  // Your instructions
std::vector<Instruction *> Instructions;
for (const MCInst &MI : Insts) {
  Instructions.push_back(IB.createInstruction(MI));
}

Pipeline P(C);
for (auto *Instr : Instructions) {
  P.execute(Instr);
}

// Get results from Views
```

**Query instruction resources:**
```cpp
InstrBuilder &IB = ...;
const MCInst &MI = ...;
std::unique_ptr<Instruction> Instr = IB.createInstruction(MI);
unsigned Latency = Instr->getLatency();
```

**Track pipeline events:**
```cpp
class MyEventListener : public HWEventListener {
  void onInstructionFetched(const Instruction *I) override { ... }
  void onInstructionRetired(const Instruction *I) override { ... }
};

Pipeline P(C);
P.addListener(std::make_unique<MyEventListener>());
```

## Dependencies

### Internal
- `MC/` - MCInst, MCInstrInfo, MCSubtargetInfo (instruction and target metadata)
- `Support/` - data structures, error handling

### Generated from Target Description
- Scheduling models (from target `.td` files, compiled to MCSchedule)
- Instruction latencies and resource requirements

<!-- MANUAL: -->
