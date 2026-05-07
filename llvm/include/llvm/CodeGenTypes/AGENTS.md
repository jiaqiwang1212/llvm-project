<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CodeGenTypes

## Purpose

Type lowering from LLVM IR types to machine-level types. Defines the machine value types (MVT) for registers and the extended value types (EVT) that combine MVT with additional semantic information. Responsible for determining how LLVM types (i64, f32, etc.) map to target register widths and alignment.

## Key Files

| File | Purpose |
|------|---------|
| `MachineValueType.h` | Machine Value Type (MVT): register-width types (i32, i64, f32, etc.) |
| `LowLevelType.h` | Generic Machine IR type representation for GlobalISel |

## For AI Agents

### Working In This Directory

**Understand the type hierarchy:**
- LLVM IR types (`Type*` from `IR/Type.h`) are abstract type descriptors
- MVT (MachineValueType) represents what fits in a register or appears in MachineInstr operands
- EVT (ExtendedValueType) wraps MVT with additional flags (signedness, legality)
- LLT (LowLevelType) is a target-independent generic representation used by GlobalISel

**Type lowering decisions:**
- TargetLowering determines how IR types map to legal MVTs
- Some IR types may expand (i64 on 32-bit target) or be legalized (i24 → i32)
- Type legality is target-specific and determined during instruction selection

**Common type widths:**
- `i1`, `i8`, `i16`, `i32`, `i64`, `i128` (and larger via custom widths)
- `f32`, `f64`, `f80`, `f128` (floating point)
- Vector types: `v4i32`, `v2f64`, etc.

### Common Patterns

**Check type legality:**
```cpp
TargetLowering &TLI = ...;
EVT VT = EVT::getEVT(IRType);
if (TLI.isTypeLegal(VT)) { ... }
```

**Get register type:**
```cpp
MVT RegisterType = TLI.getRegisterType(MyEVT);
unsigned NumRegisters = TLI.getNumRegisters(MyEVT);
```

**Convert between type representations:**
```cpp
MVT MV = MVT::i32;  // Machine value type
EVT EV = EVT(MV);   // Extended type
LLT Generic = LLT::scalar(32);  // Generic machine IR type
```

## Dependencies

### Internal
- `IR/` - LLVM IR Type* for conversion
- `CodeGen/` - TargetLowering, SelectionDAG use these types

<!-- MANUAL: -->
