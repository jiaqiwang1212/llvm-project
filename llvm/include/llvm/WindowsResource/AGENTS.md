<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# WindowsResource

## Purpose
Windows resource (.rc) file parsing and binary resource object generation. Handles icons, dialogs, strings, and other Windows resources.

## Key Files
| File | Description |
|------|-------------|
| `ResourceProcessor.h` | Main resource file processor |
| `ResourceScriptToken.h` | Lexical tokens from .rc scripts |
| `ResourceScriptTokenList.h` | Token stream representation |

## For AI Agents

### Working In This Directory
WindowsResource processes Windows resource scripts:
- **.rc file parsing**: Parse Windows resource script syntax
- **Resource types**: Handle dialogs, strings, menus, icons, bitmaps, etc.
- **Binary generation**: Create .res or embed in executables
- **Language support**: Multi-language resource strings

### Common Patterns
- Tokenize .rc script input
- Parse token stream into resource definitions
- Validate resource structure and references
- Emit binary .res or PE-embedded format

## Dependencies

### Internal
- `llvm/Support` (StringRef, Error, raw_ostream)
- `llvm/ADT` (SmallVector, StringMap)

### External
- Windows resource format specifications

<!-- MANUAL: -->
