<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithCommon

## Purpose
Provides shared utilities for Arith dialect conversion passes. Contains attribute and type conversion helpers reused across multiple Arith lowering targets (LLVM, SPIRV, etc.).

## Key Files
| File | Description |
|------|-------------|
| `AttrToLLVMConverter.h` | Shared attribute-to-LLVM conversion utilities for Arith passes |

## For AI Agents

### Working In This Directory
- Edit `AttrToLLVMConverter.h` to add shared conversion helpers used by multiple Arith target passes
- This is a utility directory, not a standalone pass; do not register passes here

## Dependencies
- Source dialect: `include/mlir/Dialect/Arith/`
- Consumers: `ArithToLLVM/`, `ArithToSPIRV/`, and other Arith conversion passes

<!-- MANUAL: -->
