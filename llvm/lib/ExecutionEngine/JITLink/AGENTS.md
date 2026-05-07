<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# JITLink

## Purpose
Low-level linker for JIT compilation. Loads object files into memory, performs relocation, and links symbols for ORC JIT v2. Supports ELF, MachO, COFF, and XCOFF formats. Replaces RuntimeDyld with a cleaner API and architecture.

## Key Files
| File | Description |
|------|-------------|
| `JITLink.cpp` | Core JITLink API and orchestration |
| `JITLinkGeneric.cpp` | Generic linking utilities |
| `JITLinkMemoryManager.cpp` | Memory management for linked code |
| `ELFLinkGraphBuilder.cpp` | ELF object file parsing and graph building |
| `MachOLinkGraphBuilder.cpp` | MachO object file parsing and graph building |
| `COFFLinkGraphBuilder.cpp` | COFF object file parsing and graph building |
| `XCOFFLinkGraphBuilder.cpp` | XCOFF object file parsing and graph building |
| `ELF.cpp` | ELF-specific linking (generic) |
| `ELF_x86_64.cpp` | ELF x86_64 relocations |
| `ELF_aarch64.cpp` | ELF AArch64 relocations |
| `ELF_x86.cpp` | ELF x86 relocations |
| `ELF_aarch32.cpp` | ELF AArch32 relocations |
| `ELF_ppc64.cpp` | ELF PPC64 relocations |
| `ELF_riscv.cpp` | ELF RISC-V relocations |
| `ELF_systemz.cpp` | ELF SystemZ relocations |
| `ELF_hexagon.cpp` | ELF Hexagon relocations |
| `ELF_loongarch.cpp` | ELF LoongArch relocations |
| `MachO.cpp` | MachO-specific linking (generic) |
| `MachO_x86_64.cpp` | MachO x86_64 relocations |
| `MachO_arm64.cpp` | MachO ARM64 relocations |
| `COFF.cpp` | COFF-specific linking |
| `COFF_x86_64.cpp` | COFF x86_64 relocations |
| `COFFDirectiveParser.cpp` | COFF linker directive parsing |
| `XCOFF.cpp` | XCOFF-specific linking |
| `XCOFF_ppc64.cpp` | XCOFF PPC64 relocations |
| `CompactUnwindSupport.cpp` | MachO compact unwind info handling |
| `EHFrameSupport.cpp` | EH frame (exception handling) support |
| `DWARFRecordSectionSplitter.cpp` | DWARF section splitting for debuggability |
| `PerGraphGOTAndPLTStubsBuilder.h` | GOT/PLT stub generation for relocations |
| `DefineExternalSectionStartAndEndSymbols.h` | Symbol definition helpers |
| `SEHFrameSupport.h` | Windows SEH frame support |
| `Architecture-specific files` | `aarch32.cpp`, `aarch64.cpp`, `hexagon.cpp`, `loongarch.cpp`, `ppc64.cpp`, `riscv.cpp`, `systemz.cpp`, `x86_64.cpp`, `x86.cpp` |

## For AI Agents

### Working In This Directory
- JITLink replaces RuntimeDyld; prefer JITLink for new JIT work.
- Each architecture (x86_64, aarch64, etc.) has dedicated relocation handling; changes must be platform-specific.
- **JITLink.cpp** is the main API; understand LinkGraph and relocation model before modifying.
- EH frame and compact unwind support are critical for debugging and exception handling; test on your target platform.
- COFFDirectiveParser handles COFF-specific linker directives; changes require COFF format knowledge.

### Dependencies

#### Internal
- Depends on `llvm/lib/IR/` (Module information)
- Depends on `llvm/lib/Object/` (ObjectFile, object format parsing)
- Depends on `llvm/lib/Support/` (Error handling, memory allocation)
- Used by `llvm/lib/ExecutionEngine/Orc/ObjectLinkingLayer.cpp`

<!-- MANUAL: -->
