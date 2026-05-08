<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IndexToLLVM Conversion

## Purpose
Lowers the Index dialect to LLVM IR dialect. Converts `index.*` arithmetic ops to their LLVM `i64` (or `i32` on 32-bit targets) equivalents, with target-width determined by the data layout.

## Key Files
| File | Description |
|------|-------------|
| `IndexToLLVM.cpp` | Conversion patterns from index ops to llvm ops with pointer-width integers |

## For AI Agents

### Working In This Directory
- `index` type maps to `i64` on 64-bit targets, `i32` on 32-bit; `LLVMTypeConverter` handles this via data layout.
- `index.add` → `llvm.add`, `index.mul` → `llvm.mul`, `index.ceildivs` → composite division sequence, etc.
- `index.casts` (index to integer and back) map to `llvm.zext`/`llvm.trunc`/`llvm.sext` as appropriate.
- Must run before `FuncToLLVM` since function signatures may carry `index` types.

## Dependencies
- Source: `mlir/Dialect/Index`
- Target: `mlir/Dialect/LLVMIR`
- Utility: `mlir/Conversion/LLVMCommon`

<!-- MANUAL: -->
