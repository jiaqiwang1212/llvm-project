<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-04-23 | Updated: 2026-04-23 -->

# TargetInfo

## Purpose
Registers BPF targets with the LLVM global `TargetRegistry`. Exposes three `Target` singletons: host-endian `bpf`, little-endian `bpfle`, and big-endian `bpfbe`. This is the dependency-free entry point that all other BPF backend libraries link against.

## Key Files

| File | Description |
|------|-------------|
| `BPFTargetInfo.h` | Declares `getTheBPFTarget()`, `getTheBPFleTarget()`, `getTheBPFbeTarget()` |
| `BPFTargetInfo.cpp` | Defines the three `Target` singletons and registers them via `RegisterTarget<>` in `LLVMInitializeBPFTargetInfo()` |
| `CMakeLists.txt` | Outputs the `BPFInfo` library; linked first before all other BPF libraries |

## For AI Agents

### Working In This Directory

- Must remain **dependency-free** relative to all other BPF subdirectories to avoid circular dependencies — it is the foundation layer.
- `LLVMInitializeBPFTargetInfo()` is invoked automatically by `LLVM_INITIALIZE_TARGET(BPF)` and `InitializeAllTargets()` macros; do not call it manually in backend code.
- `bpf` triple resolves to host endianness at runtime; `bpfle` and `bpfbe` are explicit. All three map to the same `BPFTargetMachine`.

### Testing Requirements

- `llc --version | grep bpf` should show all three triples (`bpf`, `bpfle`, `bpfbe`).

### Common Patterns

- Each `Target` global is a function-local `static` returned by reference — the standard LLVM singleton pattern.

## Dependencies

### Internal
- None — intentionally leaf-level.

### External
- `llvm/MC/TargetRegistry.h` — `RegisterTarget<>` template

<!-- MANUAL: -->
