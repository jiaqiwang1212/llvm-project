<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/Passes

## Purpose

Pass manager framework and optimization pipeline builders. Constructs compilation pipelines from individual passes, manages pass dependencies, handles instrumentation and profiling, and implements the modern pass manager.

## Key Files

| File | Description |
|------|-------------|
| `PassBuilder.cpp` | Builds optimization pipelines (-O0 to -O3) |
| `PassBuilderPipelines.cpp` | Pipeline construction for different optimization levels |
| `PassBuilderBindings.cpp` | C bindings for PassBuilder |
| `PassRegistry.def` | Pass registration definitions (metadata-driven) |
| `StandardInstrumentations.cpp` | Timing, pass printing, and standard instrumentation |
| `CodeGenPassBuilder.cpp` | Code generation pass pipeline building |
| `OptimizationLevel.cpp` | Optimization level representation and defaults |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When modifying the pass pipeline:

1. **Understanding pipeline construction**: PassBuilder::buildPerModuleDefaultPipeline() builds -O2
2. **Adding passes**: Register in PassRegistry.def, add to appropriate pipeline function
3. **Pass dependencies**: Declare required analyses via analysis registration
4. **Optimization levels**: Adjust pipeline composition for -O0, -O1, -O2, -O3, -Oz
5. **Testing**: Use llvm-lit with -debug-pass=Verbose to verify pipeline structure

### Common Patterns

- **Pipeline builders**: buildPerModuleDefaultPipeline(), buildFunctionSimplificationPipeline()
- **Pass registration**: FUNCTION_PASS, MODULE_PASS, LOOP_PASS macros
- **Analysis preservation**: Explicitly preserve analyses needed by later passes
- **Instrumentation**: Insert StandardInstrumentations for timing and debug output

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR types, modules, functions
- `llvm/include/llvm/Analysis/` — all analysis passes
- `llvm/include/llvm/Transforms/` — all transformation passes
- `llvm/lib/Analysis/` — analysis implementations
- `llvm/lib/Transforms/` — transformation implementations
- `llvm/lib/CodeGen/` — code generation passes
- `llvm/include/llvm/Passes/PassBuilder.h` — PassBuilder interface

### External
- Standard C++ library (string handling for pass names)

## Pipeline Structure

**-O0**: Minimal passes (Mem2Reg, basic cleanup)

**-O1**: Basic optimizations (InstCombine, LICM, SimplifyCFG, DCE)

**-O2**: Standard optimizations (+ inlining, GVN, loop unrolling, vectorization)

**-O3**: Aggressive optimizations (+ function specialization, IPO, partialInlining)

**-Oz**: Size optimization (+ limited unrolling, avoid expensive passes)

<!-- MANUAL: -->
