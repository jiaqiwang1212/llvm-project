<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IRDL Dialect

## Purpose
Implements the IR Definition Language (IRDL) dialect — a meta-dialect for defining other dialects dynamically at runtime. IRDL lets you describe ops, types, and attributes with their constraints using MLIR ops instead of C++ code, enabling dynamic dialect loading.

## Key Files
| File | Description |
|------|-------------|
| `IR/IRDL.cpp` | Core dialect and op implementations |
| `IR/IRDLOps.cpp` | Op-level constraint and definition ops |
| `IRDLLoading.cpp` | Loads IRDL dialect definitions and registers them as dynamic dialects |
| `IRDLSymbols.cpp` | Symbol table management for IRDL-defined ops and types |
| `IRDLVerifiers.cpp` | Verification logic for ops defined via IRDL at runtime |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | IRDL meta-ops for describing ops, types, attributes |

## For AI Agents

### Working In This Directory
- IRDL is unusual: the "IR" it defines is loaded at runtime via `IRDLLoading.cpp`, not compiled into a binary.
- `irdl.dialect`, `irdl.operation`, `irdl.type`, `irdl.attribute` ops hierarchically define a dialect's contents.
- Constraint ops (`irdl.any`, `irdl.is`, `irdl.parametric`, `irdl.any_of`) express type/attribute constraints.
- `IRDLVerifiers.cpp` uses the constraint tree to verify dynamically loaded ops at runtime.
- Use case: loading dialect extensions from MLIR files without recompiling.

### Common Patterns
- `IRDLLoading.cpp` traverses an IRDL module and calls `DynamicDialect::createDynamicOp()` for each defined op.
- Verifiers built from IRDL constraints check operand/result type conformance at op verification time.

## Dependencies
- `mlir/IR`, `mlir/IR/ExtensibleDialect`, `mlir/Dialect/Func`

<!-- MANUAL: -->
