<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TableGen

## Purpose
TableGen DSL parser, evaluator, and backend framework. Powers LLVM's declarative specification system for instruction sets, register definitions, and other structured data.

## Key Files
| File | Description |
|------|-------------|
| `TGLexer.cpp` | TableGen lexer/tokenizer |
| `TGParser.cpp` | TableGen parser and AST construction |
| `Record.cpp` | Record definition and management |
| `SetTheory.cpp` | Set algebra and expression evaluation |
| `Main.cpp` | TableGen compiler entry point |
| `Parser.cpp` | High-level parsing interface |
| `Error.cpp` | Error reporting and diagnostics |
| `TableGenBackend.cpp` | Backend base class |
| `TableGenBackendSkeleton.cpp` | Backend skeleton generator |
| `DetailedRecordsBackend.cpp` | Detailed record emitter |
| `JSONBackend.cpp` | JSON output backend |
| `StringMatcher.cpp` | String matching code generation |
| `StringToOffsetTable.cpp` | String compression tables |
| `TGTimer.cpp` | Performance profiling |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- TableGen language syntax and semantics
- Multi-pass compilation: lex → parse → evaluate → emit
- Backend extensibility for code generation
- Used by Target/ to define instructions and registers
- Used by Option/ for command-line definitions

## Dependencies

### Internal
- `llvm/lib/Support/` — utilities, error handling

<!-- MANUAL: -->
