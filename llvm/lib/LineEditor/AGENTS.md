<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LineEditor

## Purpose
Provides interactive line editing with history support for command-line tools. Wraps platform line editing libraries (libedit on Unix, Windows console API on Windows).

## Key Files
| File | Description |
|------|-------------|
| `LineEditor.cpp` | Line editor implementation with history |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Interactive command-line interface support
- History management and recall
- Platform-specific implementations (Unix/Windows)
- Used by LLVM interactive tools and debuggers

## Dependencies

### Internal
- `llvm/lib/Support/` — error handling

<!-- MANUAL: -->
