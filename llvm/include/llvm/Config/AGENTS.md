<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Config

## Purpose

The Config directory contains CMake templates that generate build-time configuration headers. These headers define compile-time constants and feature flags that control LLVM behavior: target availability, threading support, version information, architecture detection, and optional feature enablement.

Files in this directory are **CMake templates** (`.cmake` or `.in` suffix) and are **not meant to be edited directly**. The LLVM build system processes these templates and generates actual header files in the build directory.

## Key Files

| File | Description |
|------|-------------|
| `llvm-config.h.cmake` | Main config header (LLVM_VERSION_*, LLVM_NATIVE_ARCH, feature flags) |
| `config.h.cmake` | Legacy platform-specific configuration |
| `abi-breaking.h.cmake` | ABI stability enforcement macros |
| `Targets.def.in` | Target backend availability (populated by CMake) |
| `AsmParsers.def.in` | Assembly parser availability per target |
| `AsmPrinters.def.in` | Assembly printer availability per target |
| `Disassemblers.def.in` | Disassembler availability per target |
| `TargetMCAs.def.in` | Machine code analyzer availability per target |
| `TargetExegesis.def.in` | Exegesis (performance measurement) availability per target |

## For AI Agents

### Working In This Directory

When modifying Config:

1. **Edit only `.cmake` and `.in` template files**, never the generated headers
2. **Run CMake after any changes** to regenerate headers in the build directory
3. **Verify the generated header** in `build/llvm/include/llvm/Config/llvm-config.h`
4. **Use template syntax** (CMake @VAR@ substitutions) for conditional content
5. **Test on multiple platforms** if adding architecture or OS detection
6. **Document each macro** with its purpose and possible values
7. **Never hardcode version numbers**—use CMake variables (PROJECT_VERSION, etc.)

### Common Patterns

**Accessing config macros in C++:**
```cpp
#include "llvm/Config/llvm-config.h"

#if LLVM_ENABLE_THREADS
  // Multi-threaded LLVM features
#endif

#if defined(LLVM_NATIVE_ARCH)
  StringRef arch = LLVM_NATIVE_ARCH;
#endif

int major = LLVM_VERSION_MAJOR;
int minor = LLVM_VERSION_MINOR;
```

**Checking target availability:**
```cpp
#include "llvm/Config/Targets.def"

// The generated Targets.def provides LLVM_TARGET() macro for each enabled target
// Backend code iterates over these to register available targets
```

**CMake template example (`.cmake` syntax):**
```cmake
#cmakedefine LLVM_ENABLE_THREADS @LLVM_ENABLE_THREADS@
#define LLVM_VERSION_MAJOR @LLVM_VERSION_MAJOR@
#define LLVM_VERSION_STRING "@LLVM_VERSION_STRING@"
```

## Dependencies

### Internal

- CMake configuration (processed during build, not runtime)
- No runtime dependencies (header-only)

### Build-Time

- CMake 3.13+
- `llvm/CMakeLists.txt` sets variables used in templates

## Notes

**Do not commit generated files** (those in the build directory). Only template sources (in `llvm/include/llvm/Config/`) are version controlled. The build system regenerates headers on each build.

When cross-compiling, CMake respects the target triple and populates config macros accordingly. Test on the actual target platform if possible to verify correct configuration.

<!-- MANUAL: -->
