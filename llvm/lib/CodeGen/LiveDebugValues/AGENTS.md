<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LiveDebugValues

## Purpose

Propagates variable location debug information through machine code pipeline. Solves: after register allocation, spilling, and optimizations, which machine register/stack slot contains variable X at instruction Y? Enables accurate debugger variable inspection. Critical for debug experience.

## Key Files

| File | Purpose |
|------|---------|
| `LiveDebugValues.cpp` | Main pass driver, orchestrates implementations |
| `LiveDebugValues.h` | LiveDebugValues class definition |
| `VarLocBasedImpl.cpp` | Variable location tracking (legacy approach, simpler) |
| `InstrRefBasedImpl.cpp` | Instruction reference tracking (newer, more accurate) |
| `InstrRefBasedImpl.h` | InstrRefBasedImpl class definition |

## For AI Agents

### Working In This Directory

This directory handles a single, critical pass: propagating debug variable locations. Start here for:
- How debuggers know where variables are at each instruction
- Debug location tracking through optimizations
- Machine value tracking (which register/stack slot holds what)
- Live ranges of debug values

The pass runs post-register-allocation. It walks instructions and tracks which physical registers/stack slots contain which source variables.

### Common Patterns

**Variable Locations**: Each variable at each point has a location (register, stack, dead, optimized-out):
```cpp
// Variable at instruction I is in reg X / stack slot Y / dead
struct VarLocInfo {
  Register Loc;           // e.g., X86::RCX
  int StackSlot;         // or stack offset
  bool IsOptimizedOut;
};
```

**Implementation Modes**:
- **VarLocBasedImpl** (legacy): Simpler, tracks variable→location mappings directly
- **InstrRefBasedImpl** (modern): More accurate, uses instruction references and value propagation

**Liveness Computation**: Like traditional liveness analysis but on debug values:
```cpp
for each instruction I {
  for each debug value at I {
    propagate live range based on use/def
  }
}
```

**Stack Slots**: After register allocation, variables may be spilled to stack. Track both register and stack locations.

## Dependencies

### Internal
- `../` — CodeGen core (MachineFunction, MachineInstr, LiveIntervals, VirtRegMap)
- `include/llvm/CodeGen/MachineFunctionPass.h` — Pass infrastructure

### External
- `include/llvm/CodeGen/TargetRegisterInfo.h` — Target register info
- `include/llvm/CodeGen/TargetFrameLowering.h` — Stack frame info
- `IR/DebugInfoMetadata.h` — LLVM IR debug metadata (DIVariable, DILocation, etc.)

<!-- MANUAL: -->
