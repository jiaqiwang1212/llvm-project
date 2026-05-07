<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SandboxIR

## Purpose
Sandboxed IR with rollback support that wraps LLVM IR in a transaction-like interface. Enables safe exploration and testing of IR transformations with automatic rollback on failure.

## Key Files
| File | Description |
|------|-------------|
| `Context.cpp` | Sandbox context and transaction state |
| `Module.cpp` | Sandboxed module wrapper |
| `Function.cpp` | Sandboxed function wrapper |
| `BasicBlock.cpp` | Sandboxed basic block wrapper |
| `Instruction.cpp` | Sandboxed instruction wrapper |
| `User.cpp` | Sandboxed user base class |
| `Value.cpp` | Sandboxed value wrapper |
| `Type.cpp` | Sandboxed type references |
| `Constant.cpp` | Sandboxed constant values |
| `Argument.cpp` | Sandboxed function arguments |
| `Use.cpp` | Sandboxed use-def chains |
| `Tracker.cpp` | Change tracking and rollback |
| `Pass.cpp` | Sandbox-aware pass infrastructure |
| `PassManager.cpp` | Sandbox pass execution |
| `Region.cpp` | Code region abstraction |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Transaction semantics for IR changes
- Rollback capability on transformation failure
- Tracking modified values and instructions
- Wrapper pattern over core IR types
- Safe IR exploration and experimentation

## Dependencies

### Internal
- `llvm/lib/IR/` — core IR types and operations
- `llvm/lib/Pass/` — pass infrastructure

<!-- MANUAL: -->
