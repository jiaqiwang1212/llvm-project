<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# RuntimeDyld

## Purpose
Dynamic linker for MCJIT. Loads object files into memory, performs relocation, and resolves symbols. Primarily used by legacy MCJIT; ORC JIT v2 uses JITLink instead. Maintains support for ELF, MachO, and COFF formats.

## Key Files
| File | Description |
|------|-------------|
| `RuntimeDyld.cpp` | Main RuntimeDyld API |
| `RuntimeDyldImpl.h` | Implementation details |
| `RTDyldMemoryManager.cpp` | Memory management for loaded code |
| `JITSymbol.cpp` | Symbol and symbol table management |
| `RuntimeDyldChecker.cpp` | Verification and testing utilities |
| `RuntimeDyldCheckerImpl.h` | Checker implementation |
| `RuntimeDyldELF.cpp` | ELF relocation handling |
| `RuntimeDyldELF.h` | ELF-specific declarations |
| `RuntimeDyldMachO.cpp` | MachO relocation handling |
| `RuntimeDyldMachO.h` | MachO-specific declarations |
| `RuntimeDyldCOFF.cpp` | COFF relocation handling |
| `RuntimeDyldCOFF.h` | COFF-specific declarations |

## For AI Agents

### Working In This Directory
- RuntimeDyld is used by legacy MCJIT; ORC JIT v2 clients should use JITLink instead.
- Format-specific relocations are handled in RuntimeDyldELF, RuntimeDyldMachO, RuntimeDyldCOFF; changes must be platform-specific.
- Memory management is critical for correctness; coordinate with RTDyldMemoryManager.
- RuntimeDyldChecker provides testing infrastructure; add tests for new relocations.

### Dependencies

#### Internal
- Depends on `llvm/lib/Object/` (object file parsing)
- Depends on `llvm/lib/Support/` (memory allocation, error handling)
- Used by `../MCJIT/` (legacy JIT) and some external clients

<!-- MANUAL: -->
