<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Object

## Purpose
Implements object file reading and manipulation for multiple formats: ELF, COFF, Mach-O, GOFF, and WebAssembly. Provides unified interface for inspecting binary files.

## Key Files
| File | Description |
|------|-------------|
| `Binary.cpp` | Binary file abstraction base |
| `Archive.cpp` | Archive (.a) file reading |
| `ArchiveWriter.cpp` | Archive creation and writing |
| `ELFObjectFile.cpp` | ELF object file support |
| `COFFObjectFile.cpp` | COFF/PE object file support |
| `MachOObjectFile.cpp` | Mach-O object file support |
| `MachOUniversal.cpp` | Universal (fat) Mach-O support |
| `GOFFObjectFile.cpp` | GOFF (IBM) object file support |
| `IRObjectFile.cpp` | LLVM IR object file wrapping |
| `IRSymtab.cpp` | IR symbol table for LTO |
| `BBAddrMap.cpp` | Basic block address mapping |
| `BuildID.cpp` | Build ID extraction |
| `Decompressor.cpp` | Compressed section decompression |
| `DXContainer.cpp` | DirectX container format |
| `FaultMapParser.cpp` | Fault map section parsing |
| `Error.cpp` | Object file error handling |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Multi-format object file abstraction
- Symbol table and relocation reading
- Section iteration and attribute access
- Format-specific parsing strategies
- Integration with tools like llvm-objdump, llvm-readobj

## Dependencies

### Internal
- `llvm/lib/BinaryFormat/` — format specifications
- `llvm/lib/Support/` — error handling, utilities
- `llvm/lib/Bitstream/` — bitcode reading (for IR objects)

<!-- MANUAL: -->
