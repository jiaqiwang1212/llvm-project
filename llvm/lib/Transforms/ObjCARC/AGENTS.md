<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCARC

## Purpose

Objective-C automatic reference counting (ARC) optimization. Analyzes and optimizes retain/release operations on Objective-C objects, reducing memory management overhead by eliminating redundant reference count manipulations.

## Key Files

| File | Description |
|------|-------------|
| `ObjCARC.cpp` | Main ARC pass entry point and orchestration |
| `ObjCARC.h` | Core ARC data structures and utilities |
| `ObjCARCOpts.cpp` | Core ARC optimization logic |
| `ObjCARCContract.cpp` | Contraction phase (late optimization) |
| `ObjCARCExpand.cpp` | Expansion phase (early lowering) |
| `ProvenanceAnalysis.cpp` | Object provenance analysis |
| `ProvenanceAnalysis.h` | Provenance analysis interface |
| `DependencyAnalysis.cpp` | ARC dependency analysis |
| `DependencyAnalysis.h` | Dependency analysis interface |
| `PtrState.cpp` | Pointer state tracking for retain/release |
| `PtrState.h` | Pointer state data structures |
| `ARCRuntimeEntryPoints.h` | Objective-C runtime function declarations |
| `BlotMapVector.h` | Blotted map vector data structure |
| `ProvenanceAnalysisEvaluator.cpp` | Provenance analysis evaluation |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When modifying ARC optimization:

1. Understand ARC reference counting rules (retain, release, autorelease)
2. Use ProvenanceAnalysis to track object origins
3. Implement optimization in ObjCARCOpts.cpp
4. Use PtrState for tracking retain/release pairs
5. Verify safety with DependencyAnalysis
6. Test with llvm-lit on iOS/macOS code

### Common Patterns

- **Retain/release pairing**: Match retain/release pairs and eliminate redundant ones
- **Provenance tracking**: Determine where pointers originate for safety
- **Autorelease optimization**: Convert retainautorelease to autoreleaseReturnValue
- **Escape analysis**: Determine if pointers escape local scope

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR instructions, intrinsics (objc_* functions)
- `llvm/include/llvm/Analysis/ObjCARCAnalysisUtils.h` — utility functions
- `llvm/lib/Analysis/` — analysis implementations
- Objective-C runtime headers (for runtime function signatures)

### External
- Objective-C runtime (libobjc on Apple platforms)
- Standard C++ library

<!-- MANUAL: -->
