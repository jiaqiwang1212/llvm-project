<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Passes

## Purpose

The Passes directory contains the new Pass Manager infrastructure and optimization pipeline builders. This includes PassManager implementations for different IR scopes (Module, Function, Loop, CGSCC), the PassBuilder that constructs standard optimization pipelines (-O0 to -O3), and analysis management utilities.

## Key Files

| File | Description |
|------|-------------|
| `PassBuilder.h` | Builds and constructs standard optimization pipelines; parses optimization levels (-O0 to -O3) |
| `OptimizationLevel.h` | Enum definitions for optimization levels (None, O1, O2, O3, Os, Oz) |
| `StandardInstrumentations.h` | Standard instrumentation callbacks (logging, timings, IR verification) |
| `CodeGenPassBuilder.h` | Code generation pass pipeline builder |
| `MachinePassRegistry.def` | Machine-level pass registry definitions |
| `TargetPassRegistry.inc` | Target-specific pass registry includes |

## Subdirectories (if applicable)

No subdirectories; all pass management infrastructure is in this directory.

## For AI Agents

### Working In This Directory

When using or extending the pass management system:

1. **PassBuilder is the entry point** — Use `PassBuilder::buildPerModuleDefaultPipeline()`, `buildFunctionSimplificationPipeline()`, etc. to construct pipelines
2. **Register custom passes** — New passes must be registered via `PassBuilder::registerModuleAnalyses()`, `registerFunctionAnalyses()`, etc.
3. **Optimization levels matter** — The standard pipelines differ significantly by level; O2 and O3 include more aggressive optimizations
4. **Analysis management is automatic** — PassBuilder handles dependency tracking and invalidation; don't manually manage analysis lifetimes
5. **Instrumentation for debugging** — StandardInstrumentations provides logging, timing, and verification hooks

### Common Patterns

- **Pipeline construction** — Build pipelines using functional composition: `PM.addPass(Pass1()).addPass(Pass2()).addPass(...)` 
- **Conditional passes** — Passes may be conditionally added based on optimization level or target features
- **Loop passes** — Loop passes run via LoopPassManager, which automatically reruns passes if loops are modified
- **Analysis preservation** — Passes communicate which analyses they preserve via `PreservedAnalyses` set
- **Error handling** — PassBuilder uses `Error` type for pipeline parsing and construction failures

## Dependencies

### Internal

- `llvm/IR/PassManager.h` — Core PassManager implementations (ModulePassManager, FunctionPassManager, etc.)
- `llvm/Analysis/` — Analysis passes and analysis manager
- `llvm/Transforms/` — Individual transformation passes
- `llvm/Support/` — Utilities and error handling
- `llvm/ADT/` — Data structures

### External

- Target machine information (TargetMachine) for target-specific passes
- Profile data for PGO-informed optimizations

<!-- MANUAL: -->
