<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IPO

## Purpose

Inter-procedural optimizations (IPO) that analyze and transform functions across module boundaries. Includes inlining, function specialization, dead code elimination, whole-program optimizations, and link-time optimizations.

## Key Files

| File | Description |
|------|-------------|
| `IPO.cpp` | Main IPO pass dispatcher and pipeline builder |
| `Inliner.cpp` | Function inlining pass using cost analysis |
| `ModuleInliner.cpp` | Module-level inlining orchestration |
| `AlwaysInliner.cpp` | Unconditional inlining for marked functions |
| `PartialInlining.cpp` | Partial function inlining (cold path extraction) |
| `FunctionAttrs.cpp` | Function attribute inference (noalias, readonly, etc.) |
| `GlobalOpt.cpp` | Global variable and function optimizations |
| `GlobalDCE.cpp` | Dead global elimination |
| `ConstantMerge.cpp` | Constant and global variable merging |
| `DeadArgumentElimination.cpp` | Unused argument removal |
| `MergeFunctions.cpp` | Merges identical or compatible functions |
| `FunctionSpecialization.cpp` | Function specialization for constant arguments |
| `ArgumentPromotion.cpp` | Pass by-value promotion for aggregate arguments |
| `StripDeadPrototypes.cpp` | Removes unused function declarations |
| `Internalize.cpp` | Makes functions/globals internal (non-exportable) |
| `InferFunctionAttrs.cpp` | Infers function attributes from known patterns |
| `HotColdSplitting.cpp` | Splits hot/cold code paths |
| `OpenMPOpt.cpp` | OpenMP-specific optimizations |
| `WholeProgramDevirt.cpp` | Whole-program devirtualization |
| `LowerTypeTests.cpp` | Lowers type test checks (CFI) |
| `ThinLTOBitcodeWriter.cpp` | ThinLTO bitcode emission for distributed linking |
| `FunctionImport.cpp` | ThinLTO function importing across modules |
| `CrossDSOCFI.cpp` | Cross-DSO control flow integrity |
| `EmbedBitcodePass.cpp` | Embeds bitcode in object files |
| `SCCP.cpp` | Sparse conditional constant propagation |
| `Attributor.cpp` | Abstract attribute engine for IPO analysis |
| `AttributorAttributes.cpp` | Attribute implementations for Attributor |
| `SampleProfile.cpp` | Sample profiling guided optimization |
| `SampleProfileMatcher.cpp` | Profile matching for optimization |
| `SampleProfileProbe.cpp` | Instrumentation probes for sample profiling |
| `SampleContextTracker.cpp` | Context tracking for profiled functions |
| `CalledValuePropagation.cpp` | Call target propagation analysis |
| `BlockExtractor.cpp` | Block extraction for function splitting |
| `LoopExtractor.cpp` | Loop extraction for separate compilation |
| `ExpandVariadics.cpp` | Variadic function expansion |
| `ExtractGV.cpp` | Global variable extraction |
| `ElimAvailExtern.cpp` | Eliminates available externally functions |
| `GlobalSplit.cpp` | Splits global variables |
| `FatLTOCleanup.cpp` | Cleanup for Fat LTO objects |
| `BarrierNoopPass.cpp` | Utility pass for pipeline barriers |
| `Annotation2Metadata.cpp` | Converts annotations to metadata |
| `ForceFunctionAttrs.cpp` | Forces function attributes |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When adding new IPO passes:

1. Create new .cpp file for the pass
2. Implement ModulePass or CallGraphSCCPass
3. Use CallGraph analysis for function relationships
4. Analyze function attributes and safety properties
5. Transform functions while maintaining IR validity
6. Test with llvm-lit in llvm/test/Transforms/IPO/

### Common Patterns

- **Call graph analysis**: Use CallGraphAnalysis to understand caller/callee relationships
- **Function attributes**: Infer noalias, readonly, readnone, etc. using Attributor
- **Inlining cost**: Use InlineCost.h to determine if inlining is profitable
- **Dead elimination**: Track reachability to find unused functions/arguments

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR types, functions, calls
- `llvm/include/llvm/Analysis/CallGraph.h` — call graph analysis
- `llvm/include/llvm/Analysis/InlineCost.h` — inlining decisions
- `llvm/include/llvm/Transforms/IPO/Attributor.h` — attribute analysis
- `llvm/lib/Analysis/` — analysis implementations

### External
- Standard C++ library (containers for function analysis)

<!-- MANUAL: -->
