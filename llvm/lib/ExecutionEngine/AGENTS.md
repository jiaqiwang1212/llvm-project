<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ExecutionEngine

## Purpose
Core JIT (Just-In-Time) execution infrastructure for LLVM. Provides the abstract ExecutionEngine base class and implementations for different execution strategies: interpretation (Interpreter), legacy MC-based JIT (MCJIT), modern ORC JIT v2 (Orc), and dynamic linking (RuntimeDyld). Also includes profiler integrations (IntelJITEvents, OProfileJIT, PerfJITEvents).

## Key Files
| File | Description |
|------|-------------|
| `ExecutionEngine.cpp` | Abstract base class for JIT engines; main entry point for creating execution engines |
| `ExecutionEngineBindings.cpp` | C API bindings for ExecutionEngine |
| `GDBRegistrationListener.cpp` | GDB integration for debugging JIT-compiled code |
| `SectionMemoryManager.cpp` | Memory management for code sections |
| `TargetSelect.cpp` | Target selection and initialization for JIT |
| `CMakeLists.txt` | Build configuration |

## Subdirectories

- **IntelJITEvents/** — Intel JIT profiler event listener integration
- **IntelJITProfiling/** — Intel JIT profiling headers and types (ittnotify)
- **Interpreter/** — Pure IR interpreter (slow but portable)
- **JITLink/** — Low-level object file linker for JIT execution
- **MCJIT/** — Legacy MC-based JIT (use Orc for new code)
- **OProfileJIT/** — OProfile integration for profiling
- **Orc/** — Modern ORC JIT v2 (recommended JIT API)
- **PerfJITEvents/** — Linux perf integration
- **RuntimeDyld/** — Dynamic linker for MCJIT (loads object files in memory)

## For AI Agents

### Working In This Directory
- **ExecutionEngine.cpp** is the main entry point; understand the abstract ExecutionEngine interface before diving into implementations.
- Each subdirectory (Orc, MCJIT, Interpreter, RuntimeDyld, JITLink) is largely self-contained; changes to the base ExecutionEngine.cpp require coordination.
- Profiler integrations (IntelJITEvents, OProfileJIT, PerfJITEvents) are optional listeners; they should not block core execution.
- For new JIT work, use **Orc/**. MCJIT is legacy.

### Dependencies

#### Internal
- Depends on `llvm/lib/IR/` (Module, Function)
- Depends on `llvm/lib/CodeGen/` (target machine, MC layer)
- Depends on `llvm/lib/Target/` (target-specific code generation)
- Used by `llvm/tools/` (llc, lli)

<!-- MANUAL: -->
