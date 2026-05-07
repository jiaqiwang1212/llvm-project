<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TargetParser

## Purpose
Target triple parsing and CPU feature detection for all LLVM-supported architectures. Provides cross-platform host/target detection and feature set management.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Unix/` | Unix-specific host detection (see `Unix/AGENTS.md`) |
| `Windows/` | Windows-specific host detection (see `Windows/AGENTS.md`) |

## Key Files
| File | Description |
|------|-------------|
| `TargetParser.cpp` | Main target parsing interface |
| `Triple.cpp` | Target triple parsing and manipulation |
| `Host.cpp` | Host OS/architecture detection |
| `SubtargetFeature.cpp` | CPU feature handling |
| `TargetDataLayout.cpp` | Data layout string parsing |
| `AArch64TargetParser.cpp` | ARM64 architecture specifics |
| `ARMTargetParser.cpp` | ARM architecture specifics |
| `ARMTargetParserCommon.cpp` | Shared ARM/AArch64 code |
| `X86TargetParser.cpp` | x86/x64 architecture specifics |
| `PPCTargetParser.cpp` | PowerPC architecture specifics |
| `RISCVTargetParser.cpp` | RISC-V architecture specifics |
| `RISCVISAInfo.cpp` | RISC-V ISA extension parsing |
| `LoongArchTargetParser.cpp` | LoongArch architecture specifics |
| `AVRTargetParser.cpp` | AVR architecture specifics |
| `CSKYTargetParser.cpp` | CSKY architecture specifics |
| `XtensaTargetParser.cpp` | Xtensa architecture specifics |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Target triple format: arch-vendor-os-abi
- Architecture-specific CPU/feature databases
- Feature string parsing and compatibility
- Cross-compilation target specification
- Integration with CodeGen/ for code generation

## Dependencies

### Internal
- `llvm/lib/Support/` — string utilities

<!-- MANUAL: -->
