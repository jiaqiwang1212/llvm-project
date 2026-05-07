<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AggressiveInstCombine

## Purpose

Advanced instruction combining pass that applies more aggressive transformations than InstCombine, requiring program analysis such as dataflow or alias analysis to safely optimize instructions.

## Key Files

| File | Description |
|------|-------------|
| `AggressiveInstCombine.cpp` | Main pass implementation and instruction visitor |
| `AggressiveInstCombineInternal.h` | Internal helper functions and data structures |
| `TruncInstCombine.cpp` | Aggressive optimizations for truncation instructions |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When adding new aggressive transformations:

1. Add optimization logic to AggressiveInstCombine.cpp
2. Create separate .cpp files for complex instruction types (like TruncInstCombine.cpp)
3. Use helper functions from AggressiveInstCombineInternal.h
4. Verify transformations preserve semantics using dataflow/alias analysis
5. Test with llvm-lit tests in llvm/test/Transforms/AggressiveInstCombine/

### Common Patterns

- **Instruction visitor pattern**: Override visit* methods for specific IR instructions
- **Safety checks**: Use ValueTracking functions (isKnownNonZero, etc.) to verify safety
- **Pattern matching**: Use InstCombine pattern matcher or inline analysis
- **Replacement**: Use ReplaceInstUsesWith() and eraseFromParent()

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR types, instructions
- `llvm/include/llvm/Analysis/ValueTracking.h` — value property queries
- `llvm/include/llvm/Transforms/InstCombine/` — common patterns with InstCombine
- `llvm/lib/Analysis/` — analysis passes

### External
- Standard C++ library

<!-- MANUAL: -->
