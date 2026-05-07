<!-- Generated: 2026-05-07 -->

# IR — LLVM Intermediate Representation

## Purpose

Core implementation of LLVM's intermediate representation (IR) data model. This is the heart of LLVM: all passes, transforms, and optimizations operate on the objects defined here. The IR module provides the fundamental abstractions for representing programs as modules, functions, basic blocks, instructions, values, types, constants, and metadata.

## Key Files

| File | Description |
|------|-------------|
| `Module.cpp` | Module container — top-level IR unit holding functions, globals, and metadata |
| `Function.cpp` | Function definition and management |
| `BasicBlock.cpp` | Basic block (sequence of instructions with single entry/exit) |
| `Instruction.cpp` | Base Instruction class and core instruction behavior |
| `Instructions.cpp` | Concrete instruction implementations (ALU, memory, control flow, etc.) |
| `Value.cpp` | Value class — base for all IR values (instructions, constants, arguments, etc.) |
| `User.cpp` | User class — value that uses other values |
| `Use.cpp` | Use-def chain implementation for tracking value dependencies |
| `Type.cpp` | Type system: void, integer, floating-point, array, struct, pointer, function types |
| `Constants.cpp` | Constant value implementations and folding |
| `ConstantFold.cpp` | Compile-time constant expression evaluation |
| `ConstantRange.cpp` | Integer range analysis (signed/unsigned bounds) |
| `ConstantFPRange.cpp` | Floating-point range analysis |
| `ConstantRangeList.cpp` | Set of disjoint constant ranges |
| `IRBuilder.cpp` | Convenience API for constructing IR programmatically |
| `Verifier.cpp` | IR verification (consistency checks for well-formed modules) |
| `Attributes.cpp` | Function/argument attribute metadata |
| `Metadata.cpp` | Metadata nodes and tracking (non-semantic annotations) |
| `DebugInfoMetadata.cpp` | Debug info metadata (DWARF representation) |
| `DebugInfo.cpp` | Debug info analysis and utilities |
| `DIBuilder.cpp` | Convenience API for constructing debug metadata |
| `AsmWriter.cpp` | Text serialization of IR (.ll format) |
| `Core.cpp` | C API bindings for IR |
| `DataLayout.cpp` | Platform-specific type sizing and alignment |
| `Globals.cpp` | Global variables and aliases |
| `LLVMContext.cpp` | Context container (singleton for IR types and constants within a context) |
| `LLVMContextImpl.cpp`, `LLVMContextImpl.h` | Context implementation details |
| `PassManager.cpp` | New pass manager infrastructure (replaces LegacyPassManager) |
| `LegacyPassManager.cpp` | Old pass manager (deprecated but still in use) |
| `Pass.cpp` | Pass base classes |
| `PassRegistry.cpp` | Pass registration and discovery |
| `PassInstrumentation.cpp` | Pass timing and instrumentation |
| `Dominators.cpp` | Dominator tree construction (part of IR, not Analysis) |
| `Intrinsics.cpp` | Intrinsic function metadata |
| `IntrinsicInst.cpp` | Intrinsic-specific instruction subclasses |
| `InlineAsm.cpp` | Inline assembly representation |
| `Mangler.cpp` | Symbol name mangling |
| `MDBuilder.cpp` | Convenience API for constructing metadata |
| `PatternMatch.cpp` | Instruction pattern matching utilities |
| `Operator.cpp` | Operator instruction base class |
| `ReplaceConstant.cpp` | Constant replacement utilities |
| `Statepoint.cpp` | Statepoint instruction (GC support) |
| `SafepointIRVerifier.cpp` | Safepoint IR verification |
| `RuntimeLibcalls.cpp` | Runtime library call descriptors |
| `TypeFinder.cpp` | Utility to find and collect types in IR |
| `ModuleSummaryIndex.cpp` | Summary information for modules (inter-procedural analysis) |
| `Assumptions.cpp` | Assumption intrinsic tracking |
| `EHPersonalities.cpp` | Exception handling personality functions |
| `ConvergenceVerifier.cpp` | Verification of convergence semantics |
| `SSAContext.cpp` | SSA form context and utilities |
| `ProfileSummary.cpp` | Profiling data summary |
| `ProfDataUtils.cpp` | Profiling data utilities |
| `OptBisect.cpp` | Bisect mode for debugging optimization failures |
| `PrintPasses.cpp` | Pass printing infrastructure |
| `IRPrintingPasses.cpp` | Passes for printing IR |
| `LLVMRemarkStreamer.cpp` | Remark generation and streaming |
| `ReplaceConstant.cpp` | Constant value replacement |
| `AbstractCallSite.cpp` | Abstract call site abstraction |
| `BuiltinGCs.cpp` | Built-in garbage collection strategies |
| `CycleInfo.cpp` | Cycle information (generalization of loops) |
| `Comdat.cpp` | COMDAT section support |
| `DiagnosticHandler.cpp` | Diagnostic message handling |
| `DiagnosticInfo.cpp` | Diagnostic message types |
| `DiagnosticPrinter.cpp` | Diagnostic message printing |
| `DebugLoc.cpp` | Debug location tracking |
| `DebugProgramInstruction.cpp` | Debug program instructions |
| `DIExpressionOptimizer.cpp` | Debug expression optimization |
| `DroppedVariableStats.cpp` | Statistics on dropped variables |
| `DroppedVariableStatsIR.cpp` | IR version of dropped variable stats |
| `FPEnv.cpp` | Floating-point environment state |
| `GCStrategy.cpp` | Garbage collection strategy base |
| `GVMaterializer.cpp` | Global value materialization (lazy loading) |
| `MemoryModelRelaxationAnnotations.cpp` | Memory model annotations |
| `NVVMIntrinsicUtils.cpp` | NVIDIA-specific intrinsic utilities |
| `PseudoProbe.cpp` | Pseudo probe instrumentation |
| `StructuralHash.cpp` | Structural hashing of IR |
| `TypedPointerType.cpp` | Typed pointer type support |
| `ValueSymbolTable.cpp` | Symbol table for named values |
| `VectorTypeUtils.cpp` | Vector type utilities |
| `VFABIDemangler.cpp` | Vector function ABI demangling |
| `AutoUpgrade.cpp` | IR auto-upgrade for backward compatibility |
| `SymbolTableListTraitsImpl.h` | Symbol table list implementation |
| `ConstantsContext.h` | Constants context implementation |
| `AttributeImpl.h` | Attribute implementation |
| `MetadataImpl.h` | Metadata implementation |

## Subdirectories

None. This is a leaf directory under `llvm/lib/`.

## For AI Agents

### Working In This Directory

1. **Understand the value hierarchy**: All IR values inherit from `Value` (see `Value.h`). Study the class hierarchy before implementing IR-related changes.

2. **Context is required**: Most IR construction requires a `LLVMContext`. Never create IR types or constants outside a context.

3. **Use-def chains are sacred**: When modifying instruction operands, use `setOperand()` and `replaceAllUsesWith()` to maintain use-def chains. Direct pointer manipulation will break verification.

4. **Always verify IR**: After constructing or modifying IR programmatically, call `verifyModule()` or `verifyFunction()` to catch errors early.

5. **Type safety**: The type system enforces constraints (e.g., you cannot use an `i64` where `i32` is expected). Use `cast<>` and `dyn_cast<>` to navigate types safely.

6. **Metadata is optional**: Metadata (debug info, profiling, remarks) should never affect correctness. If an optimization passes with metadata but fails without, the optimization is buggy.

7. **Constants are immutable**: Never modify a constant after creation. Create a new constant instead.

8. **Dominance matters**: When analyzing control flow, understand dominator trees (in `Dominators.cpp`). A node dominates another if all paths from entry must pass through it.

### Common Patterns

**Constructing IR programmatically:**
```cpp
LLVMContext ctx;
Module M("test", ctx);
Function *F = Function::Create(FunctionType::get(...), GlobalValue::ExternalLinkage, "foo", M);
BasicBlock *BB = BasicBlock::Create(ctx, "entry", F);
IRBuilder<> Builder(BB);
Builder.CreateRet(Builder.CreateAdd(...));
verifyModule(M);  // Always verify after construction
```

**Iterating IR:**
```cpp
for (Function &F : M)
  for (BasicBlock &BB : F)
    for (Instruction &I : BB)
      // process I
```

**Replacing values:**
```cpp
Value *OldValue = ...;
Value *NewValue = ...;
OldValue->replaceAllUsesWith(NewValue);
```

**Type checking:**
```cpp
if (auto *CI = dyn_cast<CallInst>(V)) {
  // V is a call instruction
}
if (isa<Constant>(V)) {
  // V is some kind of constant
}
```

## Dependencies

### Internal

- **llvm/include/llvm/IR/** — Public headers for all IR classes
- **llvm/lib/Support/** — Utility functions (SmallVector, StringRef, etc.)
- **llvm/lib/AsmParser/** — Parses .ll files into IR (used by parser)
- **llvm/lib/Bitcode/Reader/** — Reads .bc files into IR
- **llvm/lib/Bitcode/Writer/** — Writes IR to .bc files

### External

None. IR is self-contained (no external dependencies).

### Dependents

Nearly every other LLVM component depends on IR:
- **Transforms/** — All optimization passes read and modify IR
- **Analysis/** — All analysis passes analyze IR
- **CodeGen/** — Lowers IR to machine code
- **Frontend/** — All language frontends produce IR

## Notes for Developers

- **When adding a new instruction type**: Add a subclass in `Instructions.cpp`, implement `classof()`, and update the Instruction opcode enum.
- **When adding a new metadata type**: Subclass `MDNode` in `Metadata.cpp` and handle serialization in Bitcode/AsmWriter.
- **Memory management**: IR values are typically managed by the containing Module or Function. Use `llvm::make_unique()` and `std::unique_ptr` where appropriate.
- **Backward compatibility**: Use `AutoUpgrade.cpp` to handle upgrades when IR format changes.
- **Performance**: The IR layer is heavily optimized. Small changes to `Value` or `Use` can have large performance impacts.

<!-- MANUAL: -->
