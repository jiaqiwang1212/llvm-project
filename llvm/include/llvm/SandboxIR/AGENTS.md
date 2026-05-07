<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SandboxIR

## Purpose

SandboxIR is an experimental sandboxed IR wrapper that provides a safe mutation interface over LLVM IR with rollback and tracking support. It allows transformations to be applied with automatic tracking of changes, validation, and the ability to roll back mutations if needed.

## Key Files

| File | Description |
|------|-------------|
| `Context.h` | SandboxIR context managing the lifetime of all SandboxIR values; tracks mutations via callbacks |
| `Value.h` | Base class for all SandboxIR values (operands, results) |
| `Instruction.h` | Instruction abstraction wrapping LLVM IR instructions |
| `BasicBlock.h` | Basic block abstraction |
| `Function.h` | Function abstraction |
| `Module.h` | Module abstraction |
| `Constant.h` | Constant value abstractions |
| `Type.h` | Type system wrapper |
| `Argument.h` | Function argument abstraction |
| `Use.h` | Use-def chain wrapper with mutation tracking |
| `Operator.h` | Operator instruction abstractions |
| `IntrinsicInst.h` | Intrinsic instruction specializations |
| `Tracker.h` | Mutation tracking and recording system |
| `Region.h` | Region/scope abstractions for grouped transformations |
| `Pass.h` | Pass infrastructure for SandboxIR transformations |
| `PassManager.h` | Pass manager for SandboxIR passes |

## Subdirectories (if applicable)

No subdirectories; all SandboxIR components are headers in this directory.

## For AI Agents

### Working In This Directory

When working with SandboxIR:

1. **Create a Context first** — `sandboxir::Context` wraps a single LLVM Module; all values live in a Context
2. **Register callbacks for mutation tracking** — `Context` provides callbacks for instruction creation, erasure, and moving
3. **Work at the SandboxIR level** — Don't directly access underlying LLVM IR; use SandboxIR abstractions (Function, BasicBlock, Instruction, etc.)
4. **Track changes** — The `Tracker` records all mutations and can replay or revert them
5. **Leverage rollback** — If a transformation fails, roll back mutations via Tracker without affecting original LLVM IR

### Common Patterns

- **Safe mutation** — Create instructions, add them to blocks, set operands; all tracked automatically
- **Validate transformations** — Verify IR is well-formed before committing changes
- **Batch operations** — Group related mutations for efficient tracking and validation
- **Callback-driven** — Custom callbacks can inspect or react to mutations in real-time
- **Bidirectional mapping** — SandboxIR values map to underlying LLVM IR; query mapping when needed

## Dependencies

### Internal

- `llvm/IR/` — Core LLVM IR classes (Function, BasicBlock, Instruction, Value, Type, Module)
- `llvm/ADT/` — Data structures (DenseMap, SmallVector, etc.)
- `llvm/Support/` — Utilities and assertions

### External

- None; SandboxIR is a self-contained wrapper layer

<!-- MANUAL: -->
