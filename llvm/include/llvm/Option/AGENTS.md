<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Option

## Purpose
Command-line option parsing framework powered by TableGen. Enables declarative option definition and automatic parsing, validation, and help text generation.

## Key Files
| File | Description |
|------|-------------|
| `OptTable.h` | Option table and parser |
| `Option.h` | Option definition and properties |
| `Arg.h` | Parsed command-line argument |
| `ArgList.h` | Collection of parsed arguments |
| `OptSpecifier.h` | Option identifier |
| `OptParser.td` | TableGen template for option definition |

## For AI Agents

### Working In This Directory
Option provides declarative command-line parsing:
- **OptTable**: Central registry of all options
- **TableGen-based definition**: Options defined in .td files, processed by TableGen
- **Automatic validation**: Type checking and constraint enforcement
- **Help generation**: Automatic --help text from definitions

### Common Patterns
- OptTable::ParseArgs() for parsing
- Iterating ArgList for retrieved options
- Check for option presence with hasArg()
- Get values with getLastArg() or getAllArgs()

## Dependencies

### Internal
- `llvm/Support` (StringRef, SmallVector)
- `llvm/ADT` (ArrayRef, DenseMap)

<!-- MANUAL: -->
