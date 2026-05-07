<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GlobalISel

## Purpose

Global Instruction Selection (ISel 2.0). Modern alternative to SelectionDAG for instruction selection. Operates on generic machine IR (G_ADD, G_LOAD, etc.) that is target-independent. Pipeline: IR→GenericMIR (IRTranslator) → Legalize → RegBankSelect → InstructionSelect. Preferred for newer targets; enables better optimization and performance.

## Key Files

| File | Purpose |
|------|---------|
| `GlobalISel.cpp` | Top-level GlobalISel pipeline driver |
| `IRTranslator.cpp` | Converts LLVM IR to generic machine IR (~5500 lines) |
| `Legalizer.cpp` | Legalization pass (narrow/widen ops to target-supported types) |
| `LegalizerHelper.cpp` | Helper functions for legalization (~2500 lines) |
| `LegalizeMutations.cpp` | Legalization mutation rules |
| `LegalityPredicates.cpp` | Predicates for legality decisions |
| `RegBankSelect.cpp` | Register bank selection (operand-specific register classes) |
| `InstructionSelect.cpp` | Target-independent instruction selection driver |
| `InstructionSelector.cpp` | Base InstructionSelector class for targets |
| `Combiner.cpp` | Machine IR combiner (peephole optimizations) |
| `CombinerHelper.cpp` | Combiner helper functions (~4000 lines) |
| `CombinerHelperArtifacts.cpp` | Combiner artifact matching |
| `CombinerHelperCasts.cpp` | Combiner cast operation rules |
| `CombinerHelperCompares.cpp` | Combiner comparison rules |
| `CombinerHelperVectorOps.cpp` | Combiner vector operation rules |
| `CSEInfo.cpp` | Common subexpression elimination (CSE) info |
| `CSEMIRBuilder.cpp` | CSE-aware MIR builder |
| `MachineIRBuilder.cpp` | Builder for constructing generic MIR |
| `LegalizerInfo.cpp` | Legality information (target-provided) |
| `LegacyLegalizerInfo.cpp` | Legacy legality interface |
| `InlineAsmLowering.cpp` | Inline assembly lowering to generic MIR |
| `CallLowering.cpp` | Function call lowering (ABI) |
| `GISelChangeObserver.cpp` | Observer pattern for MIR changes |
| `GISelValueTracking.cpp` | Value tracking utilities |
| `Utils.cpp` | Generic utilities |
| `Localizer.cpp` | Localization pass (moves constants closer to use) |
| `LoadStoreOpt.cpp` | Load/store optimization |
| `LostDebugLocObserver.cpp` | Tracks lost debug locations |
| `GIMatchTableExecutor.cpp` | Execution of match tables (ISel) |
| `MachineFloatingPointPredicateUtils.cpp` | FP predicate utilities |

## For AI Agents

### Working In This Directory

GlobalISel is the modern ISel path. Start here for:
- Generic machine IR opcodes (G_ADD, G_LOAD, G_BR, etc.)
- Type legalization (narrow/widen/split operations)
- Register bank selection (where operands live)
- Instruction selection rules (usually target-defined)
- Machine IR combining

Key difference from SelectionDAG: No DAG construction. Pipeline is linear and operating on MIR directly.

### Common Patterns

**Generic Opcodes**: All operations use G_* opcodes:
```cpp
G_ADD, G_SUB, G_MUL, G_SDIV, G_UDIV
G_LOAD, G_STORE
G_ZEXT, G_SEXT, G_TRUNC
G_ICMP, G_FCMP
G_BR, G_BRCOND
```

**Legalization**: Adapt operations to target capabilities:
```cpp
Legalizer legalizer(MF, Info, Observer);
// Converts e.g. G_ADD with i128 -> sequence of G_ADDs with smaller types
// or G_LOAD of unsupported type -> multiple loads
```

**Register Banks**: Specify where operands live (GPR, FPR, etc.):
```cpp
RegBankSelect regBankSelect(MF, RegisterBankInfo);
// Assigns register banks to virtual registers
```

**InstructionSelect**: Select generic ops to target machine instructions:
```cpp
InstructionSelector *selector = Target->createInstructionSelector();
selector->select(MI); // Convert G_ADD to X86 ADD, ARM ADD, etc.
```

**Combiner**: Machine IR peephole optimization (like DAGCombiner but on MIR):
```cpp
Combiner combiner(MF, CombinerInfo, Observer);
combiner.combineMachineInstrs();
```

## Dependencies

### Internal
- `../` — CodeGen core (MachineFunction, MachineInstr, MachineFunctionPass)
- `include/llvm/CodeGen/GlobalISel/` — GlobalISel headers
- `SelectionDAG/` — Some shared utilities (e.g., TargetLowering)

### External
- `include/llvm/CodeGen/MachineIRBuilder.h` — MIR construction
- `include/llvm/CodeGen/LegalizerInfo.h` — Legality info (target-provided)
- Each target's `Target/*/GlobalISel/` for:
  - Custom legality rules
  - Register bank info
  - Instruction selector (match table)
  - Call lowering

<!-- MANUAL: -->
