<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# InstCombine

## Purpose

Core peephole optimization pass that performs instruction-level pattern matching and algebraic simplification. One of LLVM's most frequently used and effective passes. Matches instruction patterns and replaces them with equivalent but more efficient sequences.

## Key Files

| File | Description |
|------|-------------|
| `InstructionCombining.cpp` | Main InstCombine pass driver and visitor dispatcher |
| `InstCombineInternal.h` | Internal helper functions, patterns, and types |
| `InstCombineAddSub.cpp` | Optimizations for add/sub instructions |
| `InstCombineMulDivRem.cpp` | Optimizations for multiplication/division/remainder |
| `InstCombineAndOrXor.cpp` | Bit manipulation optimizations |
| `InstCombineCompares.cpp` | Comparison instruction simplification |
| `InstCombineCalls.cpp` | Function call optimizations (intrinsics, libcalls) |
| `InstCombineCasts.cpp` | Casting and type conversion optimizations |
| `InstCombineLoadStoreAlloca.cpp` | Memory operation optimizations |
| `InstCombinePHI.cpp` | PHI node simplification |
| `InstCombineSelect.cpp` | Select instruction optimizations |
| `InstCombineShifts.cpp` | Bit shift optimizations |
| `InstCombineSimplifyDemanded.cpp` | Demanded bits analysis and simplification |
| `InstCombineVectorOps.cpp` | Vector instruction optimizations |
| `InstCombineNegator.cpp` | Negation pattern matching and strength reduction |
| `InstCombineAtomicRMW.cpp` | Atomic read-modify-write optimizations |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When adding new peephole optimizations:

1. Identify the instruction type being optimized
2. Add visitXXX() method to InstructionCombining.cpp or appropriate subfile
3. Use pattern matching with InstCombine::Builder
4. Call m_*() matchers for instruction patterns
5. Return nullptr if no match, or return replacement instruction
6. Test with llvm-lit in llvm/test/Transforms/InstCombine/

### Common Patterns

- **visitXXX methods**: Override for specific IR instruction types
- **m_* matchers**: Use m_Add(), m_Mul(), m_Constant(), m_Value() for pattern matching
- **Demanded bits**: Use getKnownBits(), computeKnownBits() for safety
- **Replacement**: Return new instruction or use replaceOperand()

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR instructions, constants
- `llvm/include/llvm/Analysis/ValueTracking.h` — value property queries
- `llvm/include/llvm/Analysis/InstructionSimplify.h` — simplification helpers
- `llvm/lib/Analysis/` — analysis passes
- `llvm/include/llvm/IR/PatternMatch.h` — pattern matching framework

### External
- Standard C++ library (vectors, maps for pattern storage)

<!-- MANUAL: -->
