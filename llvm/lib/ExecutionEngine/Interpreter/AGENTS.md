<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Interpreter

## Purpose
Portable LLVM IR interpreter that executes LLVM modules without code generation. Useful for testing, debugging, and portable execution on platforms without JIT support. Slower than JIT but correct and portable.

## Key Files
| File | Description |
|------|-------------|
| `Interpreter.cpp` | Main interpreter implementation |
| `Interpreter.h` | Interpreter class definition |
| `Execution.cpp` | Core execution loop and instruction handling |
| `ExternalFunctions.cpp` | Handlers for external/native function calls |

## For AI Agents

### Working In This Directory
- Interpreter maintains an execution state stack and memory model.
- **Execution.cpp** contains the instruction interpretation loop; changes here are performance-critical.
- **ExternalFunctions.cpp** bridges to native functions (strlen, malloc, etc.); maintain compatibility with C standard library.
- Test changes with simple IR programs before claiming success.

### Dependencies

#### Internal
- Depends on `llvm/lib/IR/` (Module, Function, Instruction types)
- Depends on target machine info for type sizes
- Used by `llc -interpreter` and `lli` tools

<!-- MANUAL: -->
