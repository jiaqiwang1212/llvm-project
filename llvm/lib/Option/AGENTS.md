<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Option

## Purpose
TableGen-driven command-line option parsing system. Provides a declarative way to define and parse command-line arguments with automatic help generation.

## Key Files
| File | Description |
|------|-------------|
| `Option.cpp` | Option class and option handling |
| `OptTable.cpp` | Option table and lookup |
| `Arg.cpp` | Argument value wrapper |
| `ArgList.cpp` | Command-line argument list |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- TableGen-based option definitions
- Argument parsing and validation
- Automatic help text generation
- Support for prefix matching and option grouping
- Used by clang, llvm tools, and other projects

## Dependencies

### Internal
- `llvm/lib/Support/` — string utilities, error handling
- `llvm/lib/TableGen/` — option definition parsing

<!-- MANUAL: -->
