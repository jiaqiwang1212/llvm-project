<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# WindowsDriver

## Purpose
Windows-specific compiler driver utilities. Manages MSVC toolchain integration, registry lookups, and Windows SDK discovery.

## Key Files
| File | Description |
|------|-------------|
| `MSVCPaths.h` | MSVC installation detection and path discovery |
| `MSVCSetupApi.h` | Windows Setup API for registry lookups |

## For AI Agents

### Working In This Directory
WindowsDriver provides Windows toolchain integration:
- **MSVC detection**: Find installed MSVC compiler versions
- **SDK discovery**: Locate Windows SDK and environment setup
- **Registry access**: Query Windows registry for tool paths
- **Version selection**: Choose specific compiler versions

### Common Patterns
- findVCToolChainViaRegistry() for MSVC discovery
- Fallback to environment variables (VCINSTALLDIR, etc.)
- Support for Visual Studio 2010 through current versions
- Integration with clang-cl for MSVC compatibility

## Dependencies

### Internal
- `llvm/Support` (StringRef, StringMap, raw_ostream)
- `llvm/ADT` (SmallVector)

### External
- Windows API (registry, Setup API)

<!-- MANUAL: -->
