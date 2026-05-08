<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRVCommon

## Purpose
Provides shared utilities for SPIR-V conversion passes. Contains attribute-to-LLVM conversion helpers reused across multiple SPIR-V lowering targets.

## Key Files
| File | Description |
|------|-------------|
| `AttrToLLVMConverter.h` | Shared attribute conversion utilities for SPIR-V passes |

## For AI Agents

### Working In This Directory
- Edit `AttrToLLVMConverter.h` to add shared helpers used across SPIR-V conversion passes
- This is a utility directory, not a standalone pass; do not register passes here

## Dependencies
- Source dialect: `include/mlir/Dialect/SPIRV/`
- Consumers: `SPIRVToLLVM/` and other SPIR-V conversion passes

<!-- MANUAL: -->
