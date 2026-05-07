<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-04-23 | Updated: 2026-05-07 -->

# TargetInfo

## Purpose
Registers the Cpu0 target(s) with the LLVM global `TargetRegistry`. This is the minimal entry point that must exist before any other backend component can be used. It exposes two singleton `Target` objects: one for big-endian (`cpu0`) and one for little-endian (`cpu0el`).

## Key Files

| File | Description |
|------|-------------|
| `Cpu0TargetInfo.h` | Declares `getTheCpu0Target()` and `getTheCpu0elTarget()` — return references to the two global `Target` singletons |
| `Cpu0TargetInfo.cpp` | Defines the singletons and calls `RegisterTarget<>` for both triples (`cpu0` / `cpu0el`) via `LLVMInitializeCpu0TargetInfo()` |
| `CMakeLists.txt` | Outputs the `Cpu0Info` library; linked before all other Cpu0 libraries |

## For AI Agents

### Working In This Directory

- This directory must remain **dependency-free** relative to other Cpu0 subdirectories — it is linked first and must not create circular dependencies.
- `LLVMInitializeCpu0TargetInfo()` is called automatically by LLVM's target initialization macros (`LLVM_INITIALIZE_TARGET` / `InitializeAllTargets`); do not call it manually in backend code.
- If a new Cpu0 triple or sub-target variant is added, register it here with an additional `RegisterTarget<>` call and expose a new `getTheCpu0xxxTarget()` function.

### Testing Requirements

- Verify with `llc --version | grep cpu0` — both `cpu0` and `cpu0el` should appear in the registered target list.

### Common Patterns

- One `Target` global per ISA variant, returned by value as a reference from a function-local `static`.

## Dependencies

### Internal
- None — intentionally leaf-level to avoid circular dependencies.

### External
- `llvm/MC/TargetRegistry.h` — `RegisterTarget<>` template

<!-- MANUAL: -->
