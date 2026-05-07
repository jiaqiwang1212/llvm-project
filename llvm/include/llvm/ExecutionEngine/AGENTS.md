<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ExecutionEngine

## Purpose
Just-in-time (JIT) compilation and runtime execution engines. Provides multiple execution strategies (interpreter, MCJIT, Orc V2) for dynamic code generation and execution.

## Key Files
| File | Description |
|------|-------------|
| `ExecutionEngine.h` | Base execution engine interface |
| `GenericValue.h` | Runtime value representation |
| `JITSymbol.h` | JIT symbol resolution |
| `JITEventListener.h` | Event callbacks during JIT |
| `Interpreter.h` | Bytecode interpreter |
| `MCJIT.h` | MCJIT engine (legacy) |
| `RTDyldMemoryManager.h` | Runtime dyld memory management |
| `RuntimeDyld.h` | Dynamic linking and relocation |
| `RuntimeDyldChecker.h` | Runtime dyld validation |
| `SectionMemoryManager.h` | Section-based memory management |
| `ObjectCache.h` | Compiled object caching |
| `OProfileWrapper.h` | OProfile profiler integration |

## Subdirectories (if applicable)
| Directory | Purpose |
|-----------|---------|
| `JITLink/` | Modern ELF/Mach-O linking |
| `Orc/` | Orc V2 JIT (recommended modern API) |

## For AI Agents

### Working In This Directory
ExecutionEngine provides multiple execution modes:
- **Interpreter**: Bytecode-based execution (slow, portable)
- **MCJIT**: Machine code JIT (older, simpler)
- **Orc V2**: Modern JIT with layered architecture (recommended)

Choose the execution strategy based on requirements (speed, simplicity, resource constraints).

### Common Patterns
- ExecutionEngine factory pattern (type-based selection)
- Memory manager callbacks for custom allocation
- Symbol resolution via global symbol table
- Event listeners for profiling and debugging

## Dependencies

### Internal
- `llvm/IR` (Module, Function, Type)
- `llvm/ADT` (SmallVector, DenseMap)
- `llvm/Support` (raw_ostream, Error)

<!-- MANUAL: -->
