<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TableGen

## Purpose
TableGen domain-specific language (DSL) parser and record system. Powers LLVM backend definition files (.td) for instruction sets, register files, and target-specific code generation.

## Key Files
| File | Description |
|------|-------------|
| `Record.h` | RecordKeeper, Record, and Init base classes |
| `Parser.h` | TableGen .td file parser |
| `Error.h` | Error reporting utilities |
| `TableGenBackend.h` | Base class for custom code generators |
| `SetTheory.h` | Set operations and expressions |
| `StringMatcher.h` | Efficient string matching tables |
| `DirectiveEmitter.h` | Directive (pragma) code generation |
| `CodeGenHelpers.h` | Common code generation utilities |
| `TGTimer.h` | Profiling and timing |
| `Main.h` | Entry point for TableGen tools |
| `AArch64ImmCheck.h` | ARM immediate value checking |
| `StringToOffsetTable.h` | Compact string table generation |
| `Automaton.td` | Template definitions |
| `SearchableTable.td` | Searchable table generation |
| `SetTheory.td` | Set theory template definitions |

## For AI Agents

### Working In This Directory
TableGen is LLVM's primary declarative specification system:
- **Record system**: Defines tables as Records with fields
- **Backend generation**: Custom code generators from records
- **Set theory**: Compositional record grouping via set operations
- **Parser**: Parses .td files into in-memory Record structures

### Common Patterns
- Visitor pattern for traversing Records
- Custom backends for specific code generation (instruction selectors, register info, etc.)
- Set composition for constraint expression
- Init hierarchy for typed values

## Dependencies

### Internal
- `llvm/Support` (StringRef, SmallVector)
- `llvm/ADT` (DenseMap, ArrayRef)

<!-- MANUAL: -->
