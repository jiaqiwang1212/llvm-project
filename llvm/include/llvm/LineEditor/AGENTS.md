<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LineEditor

## Purpose
Interactive line editor with command history. Provides readline/libedit-like functionality for LLVM tools and REPLs.

## Key Files
| File | Description |
|------|-------------|
| `LineEditor.h` | Interactive line editor interface |

## For AI Agents

### Working In This Directory
LineEditor provides terminal interaction capabilities:
- **Line editing**: Full line editing with cursor movement
- **History**: Command history with up/down navigation
- **Completion**: Custom completion callbacks
- **Multiline input**: Handle line continuations
- **Terminal abstraction**: Cross-platform terminal handling

### Common Patterns
- Create LineEditor instance with prompt
- Register completion callbacks
- Retrieve user input line-by-line
- Manage history automatically

## Dependencies

### Internal
- `llvm/Support` (StringRef, raw_ostream)

### External
- libedit or readline (optional, with fallback)
- System terminal API (termios on Unix, Windows console API)

<!-- MANUAL: -->
