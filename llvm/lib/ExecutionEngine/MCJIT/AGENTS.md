<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MCJIT

## Purpose
Legacy MC (Machine Code) based JIT compiler. Generates native machine code at runtime using LLVM's code generation pipeline. Deprecated in favor of ORC JIT v2. Maintained for backward compatibility only.

## Key Files
| File | Description |
|------|-------------|
| `MCJIT.cpp` | Main MCJIT implementation |
| `MCJIT.h` | MCJIT class definition |

## For AI Agents

### Working In This Directory
- **MCJIT is deprecated**; new JIT work should use `../Orc/` instead.
- This directory is maintained only for backward compatibility.
- Changes should be minimal and focused on compatibility, not performance.
- Before modifying MCJIT, verify that changes do not break existing users.

### Dependencies

#### Internal
- Depends on `../RuntimeDyld/` (dynamic linking)
- Depends on `llvm/lib/CodeGen/` (code generation)
- Historically used by `llc -jit` and older LLVM clients

<!-- MANUAL: -->
