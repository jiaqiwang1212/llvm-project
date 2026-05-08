<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/TableGen

## Purpose
Implements the MLIR TableGen library — the C++ model layer that parses and wraps TableGen records describing MLIR dialects, ops, types, attributes, interfaces, passes, and patterns. This library is used by all `mlir-tblgen` backends to interpret `.td` files and generate C++ code.

## Key Files
| File | Description |
|------|-------------|
| `Operator.cpp` | `Operator`: wraps a TableGen `def` record for an op; provides accessors for arguments, results, regions, successors, traits, summary, and ODS-generated format |
| `AttrOrTypeDef.cpp` | `AttrDef` / `TypeDef`: wraps records for attribute and type definitions; provides parameter, builder, and assembly format accessors |
| `Attribute.cpp` | `Attribute`: wraps attribute constraint records; distinguishes type constraints vs. attribute value constraints |
| `Type.cpp` | `Type`: wraps type constraint records used in op argument/result specs |
| `Interfaces.cpp` | `Interface`, `InterfaceMethod`: wraps interface definition records for op/type/attribute interfaces |
| `Dialect.cpp` | `Dialect`: wraps dialect definition records; provides namespace, C++ class name, and option accessors |
| `Pass.cpp` | `Pass`: wraps pass definition records; provides name, argument, description, and option accessors used by pass registration codegen |
| `Pattern.cpp` | `Pattern`: wraps DRR (Declarative Rewrite Rule) pattern records; provides source/result DAG trees for rewrite codegen |
| `Constraint.cpp` | `Constraint` and `Pred`: wraps predicate records used for type/attribute constraints |
| `Trait.cpp` | `Trait`: wraps trait records (e.g., `Pure`, `SameOperandsAndResultType`) |
| `Builder.cpp` | `Builder`: wraps custom builder declaration records for ops |
| `Class.cpp` | `Class` / `Struct`: wraps C++ class/struct emission helpers for TableGen backends |
| `CodeGenHelpers.cpp` | Shared code generation utilities: namespacing, indentation, guard generation |
| `Format.cpp` | `FmtContext` / `tgfmt()`: TableGen format string interpolation engine (replaces `$_builder`, `$_loc`, etc.) |
| `GenInfo.cpp` | `GenInfo` / `GenRegistration`: maps `--gen-*` flag names to generator functions; the plugin point for new backends |
| `Argument.cpp` | `Argument`: wraps op argument records (operands, attributes, properties) |
| `Region.cpp` | `Region`: wraps op region declaration records |
| `Successor.cpp` | `Successor`: wraps op successor (block) declaration records |
| `SideEffects.cpp` | `SideEffectTrait` / `MemoryEffect`: wraps side-effect modeling records |
| `EnumInfo.cpp` | `EnumAttr` / `EnumCase`: wraps enum attribute definition records for C++ enum codegen |
| `Property.cpp` | `Property`: wraps op property (non-attribute stored value) records |
| `Predicate.cpp` | `Pred` / `CPred` / `AndPred` / `OrPred`: wraps predicate tree records used in constraint verification codegen |

## For AI Agents

### Working In This Directory
- This library has no MLIR IR runtime dependency — it operates purely on `llvm::Record` objects from the TableGen parser.
- Changes here affect all `mlir-tblgen` backends; test with `cmake --build <build_dir> --target mlir-tblgen` and run `mlir/test/mlir-tblgen/`.
- When adding new ODS features (new op properties, new interface methods), add the corresponding wrapper class here first, then update the affected backends in `mlir/lib/Tools/mlir-tblgen/`.

### Common Patterns
- Wrapper classes store a `const llvm::Record *def` and provide typed accessors that interpret record fields by name.
- `tgfmt()` in `Format.cpp` is the standard way to emit template strings with variable substitution in backends.
- All backends register themselves via `GenRegistration` objects at static initialization time.

## Dependencies

### Internal
- No `mlir/lib/` runtime dependencies

### External
- `llvm/lib/TableGen` — `llvm::Record`, `llvm::RecordKeeper`
- `llvm/lib/Support` — ADT, `llvm::raw_ostream`

<!-- MANUAL: -->
