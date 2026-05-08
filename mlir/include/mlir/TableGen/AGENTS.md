<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/TableGen/

## Purpose
C++ TableGen backend library headers for `mlir-tblgen`. These headers provide the object model that TableGen backend code generators use to read and interpret MLIR `.td` files. Backends (op generators, interface generators, pass generators, etc.) instantiate these classes from the raw TableGen records to emit C++ and documentation output.

## Key Files
| File | Description |
|------|-------------|
| `Operator.h` | `Operator` — models an op definition from TableGen; provides access to operands, results, attributes, traits |
| `AttrOrTypeDef.h` | `AttrDef` / `TypeDef` — models attribute and type definitions |
| `Attribute.h` | `Attribute` — models a TableGen attribute constraint/reference |
| `Type.h` | `Type` — models a TableGen type constraint |
| `Interfaces.h` | `Interface`, `InterfaceMethod` — models interface definitions from `.td` |
| `Pass.h` | `Pass` — models a pass definition for pass doc/header generation |
| `Dialect.h` | `Dialect` — models a dialect definition record |
| `Constraint.h` | `Constraint` — base for type/attribute/region constraints |
| `Predicate.h` | `Pred` / `Predicate` — models `PredAttr` verification predicates |
| `Trait.h` | `Trait` / `NativeTrait` — models op traits |
| `Pattern.h` | `Pattern` / `DagLeaf` / `DagNode` — models rewrite pattern DAGs |
| `SideEffects.h` | `SideEffectTrait` — models memory effect declarations |
| `Class.h` | C++ class emission helpers for codegen backends |
| `Builder.h` | `Builder` — models custom op builder declarations |
| `Region.h` | `Region` — models op region declarations |
| `Successor.h` | `Successor` — models op successor declarations |
| `Format.h` | `tgfmt()` — printf-style formatter for TableGen code generation |
| `GenInfo.h` | `GenInfo` — registry entry for a TableGen backend generator |
| `GenNameParser.h` | CLI parser for selecting which generator to run |
| `CodeGenHelpers.h` | Common codegen utilities shared across backends |
| `EnumInfo.h` | `EnumAttr` / `EnumCase` — models enum attribute definitions |
| `Property.h` | `Property` — models op property declarations |
| `Argument.h` | `Argument` — models operand/attribute arguments to ops |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- These headers are only needed when writing a new `mlir-tblgen` backend (code generator).
- Backends register via `GenInfo` and are invoked by `mlir-tblgen -gen-<name>`.
- `Operator`, `AttrDef`, `TypeDef`, and `Interface` are the four most-used model classes.
- All model classes are constructed from `llvm::Record *` objects provided by TableGen.

### Common Patterns
- Backend entry: `static GenRegistration genReg("gen-foo", "Generate foo", [](RR &records, raw_ostream &os) { ... });`
- Op iteration: `for (auto *def : records.getAllDerivedDefs("Op")) { Operator op(def); ... }`
- Format strings: `tgfmt("$_op.getType()", &ctx)` substitutes `$_op`, `$_builder`, etc.

## Dependencies

### Internal
- `mlir/Support/` (LogicalResult)

### External
- `llvm/TableGen/Record.h` (TableGen record objects)
- `llvm/ADT/` (StringRef, SmallVector)

<!-- MANUAL: -->
