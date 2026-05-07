<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IR (Intermediate Representation)

## Purpose

The IR directory contains the core LLVM Intermediate Representation (IR) data structures and utilities. This is the central abstraction that represents programs in LLVM—everything from modules and functions down to individual instructions and values. All LLVM passes, optimizations, and backends work with IR abstractions defined here. No other part of LLVM is more fundamental.

The IR provides both the structural hierarchy (Module → Function → BasicBlock → Instruction → Value) and the semantic metadata (debug info, attributes, intrinsics, type system) needed to represent and transform programs.

## Key Files

| File | Description |
|------|-------------|
| `Module.h` | Top-level container for functions, global variables, and metadata |
| `Function.h` | Function declarations and definitions |
| `BasicBlock.h` | Sequence of instructions; building block of function control flow |
| `Instruction.h` | Base class for all IR instructions |
| `Instructions.h` | Concrete instruction classes (Load, Store, Add, etc.) |
| `Value.h` | Base class for all values (instructions, constants, arguments) |
| `Type.h` | LLVM type system (integers, floats, pointers, structures, arrays) |
| `DerivedTypes.h` | Composite types (functions, structures, arrays) |
| `Constants.h` | Constant values (ConstantInt, ConstantFP, ConstantArray, etc.) |
| `Constant.h` | Base class for constants |
| `GlobalValue.h` | Base for globals (functions, variables, aliases, ifuncs) |
| `GlobalVariable.h` | Global variables with optional initializers |
| `GlobalAlias.h` | Aliases to other globals |
| `GlobalIFunc.h` | Indirect functions |
| `Argument.h` | Function parameters |
| `IRBuilder.h` | Convenience API for constructing IR programmatically |
| `IRBuilderFolder.h` | Folding optimizations during IR construction |
| `PassManager.h` | Pass execution infrastructure for IR transformation |
| `LLVMContext.h` | Context object managing type uniqueness and IR state |
| `Metadata.h` | Metadata nodes (debug info, profiling, annotations) |
| `DebugInfoMetadata.h` | Debug info metadata structures |
| `DIBuilder.h` | Convenience API for constructing debug info |
| `Verifier.h` | Validates IR structure and type safety |
| `PatternMatch.h` | Pattern matching utilities for instruction inspection |
| `CFG.h` | Control flow graph analysis |
| `Dominators.h` | Dominance relationship analysis |
| `CycleInfo.h` | Cycle detection in control flow |
| `InstVisitor.h` | Visitor pattern for traversing and operating on instructions |
| `Use.h` | Use-def chains connecting values to their users |
| `User.h` | Base class for values that use other values |
| `OperandTraits.h` | Traits for managing operands of instructions |
| `DataLayout.h` | Target-specific data layout (sizes, alignments, offsets) |
| `CallingConv.h` | Calling convention identifiers |
| `Attributes.h` | Function and parameter attributes |
| `InlineAsm.h` | Inline assembly expressions |
| `IntrinsicInst.h` | Intrinsic function calls (LLVM-specific operations) |
| `Intrinsics.h` | Intrinsic function declarations and properties |

## For AI Agents

### Working In This Directory

When modifying IR headers:

1. Understand the value hierarchy: Value → (Constant, Instruction, Argument, Use, etc.)
2. Changes to core types (Value, Type, Instruction) affect all of LLVM—tread carefully
3. IR must be valid after construction: use Verifier.h to check
4. Use InstVisitor.h pattern when writing code that walks instruction hierarchies
5. Debug info changes must not break the DIBuilder.h contract
6. Do not add arbitrary fields to frequently-allocated types (Instruction, BasicBlock, Value)—use Metadata instead
7. Intrinsic additions require coordination with backend code generators

### Common Patterns

**Constructing IR:**
```cpp
LLVMContext ctx;
Module M("mymodule", ctx);
Function *F = Function::Create(FunctionType::get(...), GlobalValue::ExternalLinkage, "foo", M);
BasicBlock *BB = BasicBlock::Create(ctx, "entry", F);
IRBuilder<> builder(BB);
builder.CreateAdd(lhs, rhs);
```

**Pattern matching on instructions:**
```cpp
if (auto *BI = dyn_cast<BinaryOperator>(I)) {
  if (match(BI, m_Add(m_Value(lhs), m_Value(rhs)))) {
    // Handle adds
  }
}
```

**Iterating instructions:**
```cpp
for (auto &BB : *F) {
  for (auto &I : BB) {
    // Process instruction I
  }
}
```

**Type checking:**
```cpp
if (isa<IntegerType>(val->getType())) { ... }
if (val->getType()->isPointerTy()) { ... }
```

## Dependencies

### Internal

- `ADT/` — SmallVector, DenseMap, StringRef, ArrayRef for collections
- `Support/` — Error handling, raw_ostream, Casting, LLVM_DEBUG macros
- `Config/` — llvm-config.h for build-time settings

### External

- Standard library (C++17)

<!-- MANUAL: -->
