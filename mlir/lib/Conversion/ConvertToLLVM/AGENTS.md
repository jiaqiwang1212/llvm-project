<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ConvertToLLVM Conversion

## Purpose
Provides the umbrella pass and interface for converting to LLVM IR dialect. Defines `ToLLVMConversionInterface` that dialects implement to register their LLVM conversion patterns, and the `ConvertToLLVM` pass that discovers and applies all registered conversions.

## Key Files
| File | Description |
|------|-------------|
| `ConvertToLLVMPass.cpp` | The `--convert-to-llvm` pass: discovers all `ToLLVMConversionInterface` impls and applies them |
| `ToLLVMInterface.cpp` | `ToLLVMConversionInterface` dialect interface definition |

## For AI Agents

### Working In This Directory
- `ToLLVMConversionInterface` is implemented by dialects that want to participate in the unified LLVM lowering.
- The pass queries all loaded dialects for this interface and calls `populateLLVMConversionPatterns()` on each.
- This replaces the older approach of explicitly composing multiple `--convert-X-to-llvm` passes.
- New dialects targeting LLVM should implement `ToLLVMConversionInterface` to integrate with this unified pass.

## Dependencies
- Target: `mlir/Dialect/LLVMIR`
- All source dialects that implement `ToLLVMConversionInterface`

<!-- MANUAL: -->
