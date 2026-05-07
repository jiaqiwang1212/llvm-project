<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# InterfaceStub

## Purpose
ELF interface stub (IFS) generation and processing. Creates minimalist .ifs files for binary-only libraries without exposing full implementation details.

## Key Files
| File | Description |
|------|-------------|
| `IFSHandler.h` | IFS file reader/writer |
| `IFSStub.h` | IFS data structures |
| `ELFObjHandler.h` | ELF object file extraction |

## For AI Agents

### Working In This Directory
Interface stubs enable header-free binary compatibility checking:
- **IFS format**: Text-based format for library interfaces
- **Symbol extraction**: Extract exported symbols from ELF objects
- **Stubification**: Generate minimal .ifs from full binaries
- **ABI compatibility**: Verify library interface stability

### Common Patterns
- Read ELF object for symbol information
- Generate IFS from symbol list
- Validate library compatibility via IFS comparison
- Used in package management and cross-compilation

## Dependencies

### Internal
- `llvm/Support` (StringRef, Error)
- `llvm/Object` (ObjectFile)
- `llvm/ADT` (StringSet, SmallVector)

<!-- MANUAL: -->
