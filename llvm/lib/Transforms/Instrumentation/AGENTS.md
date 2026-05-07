<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Instrumentation

## Purpose

Code instrumentation passes for sanitizers, profiling, coverage analysis, and runtime checking. Instruments IR to detect bugs (memory errors, data races, undefined behavior) and collect execution metrics.

## Key Files

| File | Description |
|------|-------------|
| `AddressSanitizer.cpp` | AddressSanitizer (ASan) instrumentation for heap/stack overflow detection |
| `MemorySanitizer.cpp` | MemorySanitizer (MSan) instrumentation for uninitialized memory detection |
| `ThreadSanitizer.cpp` | ThreadSanitizer (TSan) instrumentation for data race detection |
| `HWAddressSanitizer.cpp` | Hardware-based AddressSanitizer using pointer tagging |
| `GCOVProfiling.cpp` | GCOV code coverage instrumentation |
| `InstrProfiling.cpp` | Instrumentation for PGO (Profile-Guided Optimization) |
| `SanitizerCoverage.cpp` | Sanitizer code coverage and feedback instrumentation |
| `DataFlowSanitizer.cpp` | DataFlowSanitizer (DFSan) dynamic data flow tracking |
| `RealtimeSanitizer.cpp` | RealtimeSanitizer for real-time constraint violations |
| `ControlHeightReduction.cpp` | Control height reduction for sanitizer overhead |
| `BoundsChecking.cpp` | Bounds checking for array/buffer access |
| `CGProfile.cpp` | Call graph profiling instrumentation |
| `IndirectCallPromotion.cpp` | Indirect call promotion for PGO |
| `KCFI.cpp` | Kernel Control Flow Integrity instrumentation |
| `MemProfInstrumentation.cpp` | MemProf memory profiling instrumentation |
| `MemProfUse.cpp` | MemProf data consumption and optimization |
| `NumericalStabilitySanitizer.cpp` | Numerical stability checking instrumentation |
| `PGOCtxProfFlattening.cpp` | PGO context profiling flattening |
| `PGOCtxProfLowering.cpp` | PGO context profiling lowering |
| `PGOForceFunctionAttrs.cpp` | Function attribute inference from PGO data |
| `PGOInstrumentation.cpp` | Profile-guided optimization instrumentation |
| `PGOMemOPSizeOpt.cpp` | Memory operation size optimization from PGO |
| `SanitizerBinaryMetadata.cpp` | Sanitizer metadata for binary instrumentation |
| `TypeSanitizer.cpp` | Type checking for sanitizers |
| `AllocToken.cpp` | Allocation token helpers for heap checking |
| `BlockCoverageInference.cpp` | Block coverage inference for optimization |
| `ValueProfileCollector.cpp` | Value profiling data collection |
| `LowerAllowCheckPass.cpp` | Lowering of allow-check constructs |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When adding new instrumentation passes:

1. Determine instrumentation type (detection vs profiling)
2. Create new .cpp file for the pass
3. Implement FunctionPass or ModulePass
4. Insert instrumentation calls at safe IR points (before/after instructions)
5. Link runtime library callbacks (in compiler-rt)
6. Test with llvm-lit and compiler-rt tests

### Common Patterns

- **Instruction scanning**: Iterate functions/blocks, identify instrumentation points
- **Callback insertion**: Call llvm.dbg.* or sanitizer-specific intrinsics
- **Shadow memory**: AddressSanitizer uses shadow memory for heap/stack tracking
- **Metadata preservation**: Maintain debug info and source location metadata

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR instructions, intrinsics
- `llvm/include/llvm/Analysis/TargetLibraryInfo.h` — library function information
- `llvm/include/llvm/Transforms/Instrumentation/` — pass interfaces
- `llvm/lib/Analysis/` — analysis passes
- `compiler-rt/lib/` — runtime support libraries

### External
- compiler-rt (for ASan, MSan, TSan, etc. runtimes)
- LLVM IR intrinsics (llvm.memcpy, llvm.dbg.*)

<!-- MANUAL: -->
