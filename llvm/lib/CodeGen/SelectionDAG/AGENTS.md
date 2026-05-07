<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SelectionDAG

## Purpose

SelectionDAG instruction selection pipeline. Converts LLVM IR to machine IR via a directed acyclic graph (DAG) intermediate representation. Legacy ISel path (being replaced by GlobalISel for new targets). Pipeline: IR→SelectionDAG (SelectionDAGBuilder) → Optimize (DAGCombiner) → Legalize (LegalizeDAG + Type legalization) → Select → Emit (InstrEmitter).

## Key Files

| File | Purpose |
|------|---------|
| `SelectionDAG.cpp` | SelectionDAG data structure and operations |
| `SelectionDAGBuilder.cpp` | IR→DAG lowering (~13000 lines, largest file) |
| `SelectionDAGBuilder.h` | SelectionDAGBuilder class definition |
| `DAGCombiner.cpp` | DAG optimizations (peephole, algebraic simplification) |
| `SelectionDAGISel.cpp` | ISel framework and pattern matching engine |
| `FastISel.cpp` | Fast path ISel (O(n) for quick compilation) |
| `InstrEmitter.cpp` | DAG→MachineInstr emission |
| `InstrEmitter.h` | InstrEmitter class definition |
| `LegalizeDAG.cpp` | Operation legalization (expand unsupported ops) |
| `LegalizeTypes.cpp` | Base type legalization |
| `LegalizeTypes.h` | Type legalization class definition |
| `LegalizeFloatTypes.cpp` | Floating-point type legalization |
| `LegalizeIntegerTypes.cpp` | Integer type legalization |
| `LegalizeVectorOps.cpp` | Vector operation legalization |
| `LegalizeVectorTypes.cpp` | Vector type legalization |
| `LegalizeTypesGeneric.cpp` | Generic type conversion helpers |
| `ScheduleDAGSDNodes.cpp` | Post-legalization scheduling (~1500 lines) |
| `ScheduleDAGSDNodes.h` | ScheduleDAGSDNodes class definition |
| `ScheduleDAGFast.cpp` | Fast scheduler (O(n)) |
| `ScheduleDAGRRList.cpp` | Round-robin list scheduling |
| `ScheduleDAGVLIW.cpp` | VLIW scheduling |
| `ResourcePriorityQueue.cpp` | Resource-aware priority queue for scheduling |
| `SDNodeInfo.cpp` | SelectionDAG node info and properties |
| `FunctionLoweringInfo.cpp` | Function lowering context |
| `StatepointLowering.cpp` | GC statepoint lowering |
| `StatepointLowering.h` | StatepointLowering class definition |
| `SelectionDAGAddressAnalysis.cpp` | Address calculation analysis |
| `SelectionDAGDumper.cpp` | DAG printing and visualization |
| `SelectionDAGPrinter.cpp` | DAG printing for debugging |
| `SelectionDAGTargetInfo.cpp` | Target-specific SelectionDAG info |
| `TargetLowering.cpp` | Target lowering decisions (operation legality, cost) |
| `MatchContext.h` | Pattern matching context |
| `SDNodeDbgValue.h` | Debug value representation in SelectionDAG |

## For AI Agents

### Working In This Directory

SelectionDAG is the legacy ISel pipeline. Start here for:
- SelectionDAG construction (DAG nodes, operands, chains)
- IR lowering to DAG
- DAG optimization (combining)
- Type legalization (for unsupported types)
- Instruction selection pattern matching
- Scheduling

Key concept: Everything is a DAG node (operations, values, control dependencies). Unlike GlobalISel which is linear, SelectionDAG builds an explicit graph.

### Common Patterns

**DAG Nodes**: Every value and operation is a DAG node:
```cpp
enum NodeType {
  ISD::ADD, ISD::SUB, ISD::MUL, ISD::DIV,
  ISD::LOAD, ISD::STORE,
  ISD::ZEXT, ISD::SEXT, ISD::TRUNC,
  // ... ~200 node types total
};

SDValue node = DAG.getNode(ISD::ADD, dl, VT, LHS, RHS);
```

**SelectionDAGBuilder**: Walks IR instructions and constructs DAG:
```cpp
SDValue SelectionDAGBuilder::visit(const Instruction &I) {
  switch(I.getOpcode()) {
    case Instruction::Add:
      return DAG.getNode(ISD::ADD, ...);
    // ... many cases ...
  }
}
```

**DAGCombiner**: Peephole optimization on DAG:
```cpp
// Combines patterns like: (add (shl X, 2), X) -> (shl X, 2) + X
// Or: (zext (sext X)) -> sext X (redundant extend)
```

**Type Legalization**: Adapts unsupported types:
```cpp
// i128 add -> two i64 adds with carry
// f80 on targets without f80 -> f64 or extend/truncate
```

**Pattern Matching**: InstructionSelector matches DAG patterns to machine instructions:
```cpp
// TableGen-generated match tables from .td files
// Patterns like: (add $a, $b) -> ADD reg, reg
```

**Scheduling**: Orders DAG nodes respecting dependencies and resource constraints:
```cpp
ScheduleDAGSDNodes sched(DAG);
sched.schedule(); // Returns ordered list of SDNodes for emission
```

## Dependencies

### Internal
- `../` — CodeGen core (MachineFunction, MachineInstr, after DAG→MIR emission)
- `include/llvm/CodeGen/SelectionDAG.h` — DAG header
- `include/llvm/CodeGen/TargetLowering.h` — Target lowering hooks (operation legality, cost)

### External
- `IR/` — LLVM IR types and Values (pre-ISel input)
- `Analysis/` — AliasAnalysis, LoopInfo, etc. (for optimization context)
- Each target's `Target/*/SelectionDAGISel.cpp` for:
  - Custom lowering hooks (LowerOperation, ...)
  - Pattern matching rules (from .td file)
  - Scheduling info

<!-- MANUAL: -->
