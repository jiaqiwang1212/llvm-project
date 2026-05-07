<!-- Generated: 2026-05-07 -->

# Demangle — Symbol Name Demangling

## Purpose

Converts mangled C++ and other language symbol names back to human-readable form. Mangled names are produced by compilers to encode type information, namespaces, and function signatures in symbol names (required for the linker). Demangling reverses this process, allowing tools like debuggers, profilers, and objdump to display readable names.

Supports multiple mangling schemes: Itanium (GCC, Clang), Microsoft (MSVC), Rust, D, and Swift.

## Key Files

| File | Description |
|------|-------------|
| `Demangle.cpp` | Main demangling dispatcher (dispatches to language-specific demangler) |
| `ItaniumDemangle.cpp` | Itanium (GCC/Clang) C++ name demangling |
| `MicrosoftDemangle.cpp` | Microsoft (MSVC) C++ name demangling |
| `MicrosoftDemangleNodes.cpp` | AST node classes for Microsoft demangling |
| `RustDemangle.cpp` | Rust symbol name demangling |
| `DLangDemangle.cpp` | D language symbol name demangling |

## Subdirectories

None. This is a leaf directory under `llvm/lib/`.

## For AI Agents

### Working In This Directory

1. **Language-specific schemes**: Each language has its own mangling scheme. Understand the target language before modifying demangling logic.

2. **Standards compliance**: Itanium demangling follows the Itanium C++ ABI specification. Microsoft demangling follows MSVC's scheme. Consult specs when adding support.

3. **Error handling**: Malformed mangled names must fail gracefully. Never crash on invalid input.

4. **Performance**: Demangling is used frequently (in tools like llvm-objdump). Use efficient algorithms and avoid redundant parsing.

5. **Canonical form**: Some mangled names can be represented multiple ways. Decide whether to normalize or preserve structure.

6. **Testing**: Test with real mangled names from production binaries. Use `nm`, `readelf`, or `nm -C` as reference.

### Common Patterns

**Dispatching to language-specific demangler:**
```cpp
// In Demangle.cpp
std::string demangle(std::string_view MangledName) {
  if (MangledName.startswith("_Z")) {
    return itaniumDemangle(MangledName);
  } else if (MangledName.startswith("?")) {
    return microsoftDemangle(MangledName);
  } else if (MangledName.startswith("_R")) {
    return rustDemangle(MangledName);
  }
  return MangledName;  // Unknown format, return as-is
}
```

**Parsing Itanium encoding:**
```cpp
// In ItaniumDemangle.cpp
// Itanium names follow grammar:
// <mangled-name> ::= _Z<encoding>
// <encoding> ::= <function name><bare function type>
//             |  <data name>
// ... (complex recursive descent)
```

**Handling encoding errors:**
```cpp
if (isInvalid()) {
  return "???";  // Unknown or malformed
}
```

## Dependencies

### Internal

- **llvm/lib/Support/** — String and data utilities
- **llvm/include/llvm/Demangle/Demangle.h** — Public demangling header

### External

- Mangling scheme specifications (Itanium ABI, MSVC ABI, etc.)

### Dependents

- **llvm/tools/llvm-objdump** — Displays demangled symbol names
- **llvm/tools/llvm-readelf** — Displays demangled symbol names
- **llvm/tools/llvm-nm** — Displays demangled symbol names
- **llvm/tools/llvm-gsymutil** — Demangling for debug info
- **Debuggers and profilers** — Use demangling for symbol display

## Notes for Developers

- **Itanium standard**: The Itanium C++ ABI (https://itanium.org/cxx-abi/) defines the C++ mangling scheme used by GCC and Clang. It's the most common scheme.
- **Microsoft scheme**: MSVC uses a different mangling scheme. Refer to MSDN documentation or reverse-engineer from MSVC binaries.
- **Rust mangling**: Rust uses `_R`-prefixed names with a custom encoding. See Rust RFC 2603.
- **Error messages**: When demangling fails, preserve the original mangled name rather than returning garbage.
- **Performance**: Demangling can be expensive for long names. Consider caching results if used in hot paths.
- **Testing**: Test with:
  - Real binaries from GCC, Clang, and MSVC
  - Edge cases (very deep nesting, many template parameters)
  - Malformed names (ensure no crashes)

## Example Usage

```cpp
#include "llvm/Demangle/Demangle.h"

std::string mangled = "_ZN5MyApp3FooEi";
std::string demangled = llvm::demangle(mangled);
// demangled = "MyApp::Foo(int)"

std::string rustMangled = "_RNvMs0_NtCs123_4core3numNtB2_5Testi32T";
std::string rustDemangled = llvm::demangle(rustMangled);
// rustDemangled = "core::num::i32::test"
```

<!-- MANUAL: -->
