<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-04-23 | Updated: 2026-04-23 -->

# GISel

## Purpose
GlobalISel pipeline components for BPF. Provides an alternative instruction-selection path alongside the legacy DAG ISel, using LLVM's generic machine instructions (G-MIR). `BPFSubtarget` owns all four GISel components as `unique_ptr` and vends them through the standard `getCallLowering()` / `getInstructionSelector()` / `getLegalizerInfo()` / `getRegBankInfo()` hooks.

## Key Files

| File | Description |
|------|-------------|
| `BPFCallLowering.h/.cpp` | Lowers LLVM IR calls to generic `G_` call instructions; implements `lowerReturn`, `lowerFormalArguments`, `lowerCall` |
| `BPFLegalizerInfo.h/.cpp` | Defines which generic operations are legal on BPF and how to legalize illegal ones |
| `BPFInstructionSelector.cpp` | Selects generic instructions to concrete BPF `MachineInstr`; registered via `createBPFInstructionSelector` |
| `BPFRegisterBankInfo.h/.cpp` | Describes register banks for BPF (single GPR bank); derives from `BPFGenRegisterBankInfo` |
| `BPFRegisterBanks.td` | TableGen definitions for BPF register banks |

## For AI Agents

### Working In This Directory

- **Layering**: GISel components must only include `BPFISelLowering.h` (for `BPFTargetLowering`), `BPFSubtarget.h`, and `MCTargetDesc/BPFMCTargetDesc.h`. Do not include CodeGen DAG-ISel headers.
- **Legalizer first**: when adding a new generic operation, add a legality rule in `BPFLegalizerInfo.cpp` before adding a selection rule in `BPFInstructionSelector.cpp`. An operation not covered by the legalizer will crash at compile time.
- **Register banks**: BPF has one register bank (GPR). `BPFRegisterBankInfo` maps all virtual registers to it. Changes to register classes in `BPFRegisterInfo.td` may require corresponding updates here.
- **GISel vs DAG ISel**: both pipelines coexist. `BPFTargetMachine::createPassConfig` chooses GISel when `EnableGlobalISel` is set. New lowering logic that should work in both pipelines should be implemented in both `BPFISelLowering` (DAG) and `BPFCallLowering`/`BPFInstructionSelector` (GISel).
- `BPFRegisterBanks.td` generates `BPFGenRegisterBank.inc`; use `GET_REGBANK_DECLARATIONS` and `GET_TARGET_REGBANK_CLASS` include guards as shown in `BPFRegisterBankInfo.h`.

### Testing Requirements

- Enable GISel with `-global-isel` flag to `llc` for BPF target testing.
- Tests in `llvm/test/CodeGen/BPF/GlobalISel/`.

### Common Patterns

- Each GISel component is constructed in `BPFSubtarget::initializeSubtargetFeatures` and stored in a `unique_ptr`.
- `BPFInstructionSelector` uses `MachineIRBuilder` for instruction materialization.

## Dependencies

### Internal
- `../BPFISelLowering.h` — `BPFTargetLowering` consumed by `BPFCallLowering`
- `../MCTargetDesc/BPFMCTargetDesc.h` — register enums for `BPFRegisterBankInfo`

### External
- `llvm/CodeGen/GlobalISel/CallLowering.h`
- `llvm/CodeGen/GlobalISel/LegalizerInfo.h`
- `llvm/CodeGen/GlobalISel/InstructionSelector.h`
- `llvm/CodeGen/RegisterBankInfo.h`
- `BPFGenRegisterBank.inc` — TableGen-generated; do not edit directly

<!-- MANUAL: -->
