<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transforms

## Purpose

The Transforms directory contains IR transformation passes that modify the LLVM intermediate representation to improve code quality, reduce code size, improve performance, or prepare IR for code generation. Organized by optimization scope: instruction-level (InstCombine), scalar optimizations, interprocedural optimizations, and vectorization.

## Key Files

| File | Description |
|------|-------------|
| `CFGuard.h` | Control Flow Guard transformation for Windows security |
| `IPO.h` | Interprocedural optimization header with common exports |
| `Scalar.h` | Scalar optimization header with common exports |
| `Utils.h` | Utility transformations header with common exports |

## Subdirectories (if applicable)

| Directory | Purpose |
|-----------|---------|
| `AggressiveInstCombine/` | Aggressive peephole optimizations and instruction combining |
| `InstCombine/` | Core instruction combining and canonicalization passes |
| `Scalar/` | Scalar optimization passes (ADCE, GVN, LICM, LoopUnroll, GVNExpression, etc.) |
| `IPO/` | Interprocedural optimizations (inlining, function specialization, argument promotion) |
| `Vectorize/` | Vectorization passes (LoopVectorize, SLPVectorize, LoopVectorizationLegality) |
| `Utils/` | Utility passes and helper functions (LoopUtils, ValueMapper, InlineFunction, etc.) |
| `Instrumentation/` | Instrumentation passes (PGO, sanitizers, etc.) |
| `Coroutines/` | Coroutine support transformations |
| `HipStdPar/` | HIP standard parallelization transformations |
| `ObjCARC.h` | Objective-C automatic reference counting optimizations |

## For AI Agents

### Working In This Directory

When implementing or extending transforms:

1. **Understand the pass hierarchy** — New Pass Manager (Passes/) vs old Pass Manager; most new code uses new PM
2. **IR preservation** — Specify which analyses are invalidated by your pass via `PreservedAnalyses` 
3. **Visit patterns** — Most passes iterate over IR using standard visitor patterns (walk basic blocks, instructions, etc.)
4. **Apply one transform at a time** — Each pass should do one thing well; composition is handled by PassManager
5. **Test on diverse IR** — Transformations must handle SSA form, phi nodes, unreachable code, and edge cases

### Common Patterns

- **InstCombine pattern** — Match instruction patterns, create new instructions, erase old ones; the pass runs to fixed point
- **Loop passes** — Iterate over loop tree from outermost to innermost; use LoopPassManager for automatic rerun
- **CFG-modifying passes** — Invalidate dominators and loops when modifying CFG; use DominatorTree and LoopInfo updaters
- **Incremental IR mutation** — Most transforms build new IR incrementally rather than wholesale replacement
- **Early exits** — Check if transform is applicable before expensive analysis (common in passes like ADCE, GVN)

## Dependencies

### Internal

- `llvm/IR/` — Core IR classes (Function, BasicBlock, Instruction, Value, Type, etc.)
- `llvm/Analysis/` — Analyses used by transforms (DominatorTree, LoopInfo, ScalarEvolution, etc.)
- `llvm/Passes/` — New Pass Manager infrastructure
- `llvm/ADT/` — Data structures (DenseMap, SmallVector, GraphTraits, etc.)
- `llvm/Support/` — Utilities and data structures

### External

- Target-specific information may be queried via TargetTransformInfo (TTI)
- Some passes depend on profile data or command-line options

<!-- MANUAL: -->
