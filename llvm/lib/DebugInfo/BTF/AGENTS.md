<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# BTF

## Purpose
BPF Type Format (BTF) debug information reader. Used for kernel BPF program debugging and type information on Linux. Provides compact type and symbol information for eBPF programs.

## Key Files
| File | Description |
|------|-------------|
| `BTFParser.cpp` | BTF format parsing and loading |
| `BTFContext.cpp` | BTF context and type information management |

## For AI Agents

### Working In This Directory
- BTF is a Linux/kernel-specific format; changes should be tested on Linux systems.
- Coordinate with kernel BTF documentation when extending format support.
- BTF is simpler than DWARF; focus on correctness over optimization.

### Dependencies

#### Internal
- Depends on `llvm/lib/Object/` (object file parsing)
- Depends on `llvm/lib/Support/` (error handling)
- Used by Linux kernel debugging tools and BPF verifier

<!-- MANUAL: -->
