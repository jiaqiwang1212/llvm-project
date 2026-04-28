# Cpu0 Backend — Section 2.4.1: Machine ID and Relocation Records

## Purpose

This step registers the Cpu0 tutorial architecture with LLVM's core infrastructure.
Before any Cpu0-specific backend code can be compiled or linked, LLVM must be able to:

1. Identify the architecture by name (`cpu0`, `cpu0el`, `cpu0eb`) in a target triple.
2. Emit and parse ELF object files tagged with the Cpu0 machine ID (`EM_CPU0 = 999`).
3. Decode the 24 Cpu0-specific ELF relocation types by name.
4. Report the correct data layout string for big- and little-endian variants.

These are the minimum hooks required by the rest of LLVM before a target backend can
participate in the compilation pipeline.

---

## Files Modified

### `llvm/include/llvm/TargetParser/Triple.h`

**What:** Added `cpu0` (big-endian) and `cpu0el` (little-endian) to the `ArchType` enum,
between `csky` and `dxil`.

**Why:** Every LLVM target must have an entry in this central enum. All target-triple
parsing, feature queries, and codegen routing keys off `ArchType`. Without this entry
the string `"cpu0"` cannot be parsed into a valid triple.

---

### `llvm/lib/TargetParser/Triple.cpp`

**What:** Added `cpu0`/`cpu0el` handling to eleven switch-statement sites:

| Function | Change |
|---|---|
| `getArchTypeName()` | Returns `"cpu0"` / `"cpu0el"` |
| `getArchTypePrefix()` | Both variants share prefix `"cpu0"` |
| `getArchTypeForLLVMName()` | Parses `"cpu0"`, `"cpu0eb"` → `cpu0`; `"cpu0el"` → `cpu0el` |
| `parseArch()` | Same parsing for target-triple strings |
| `getDefaultFormat()` | Default object format is ELF |
| `getArchPointerBitWidth()` | 32-bit pointer width |
| `get32BitArchVariant()` | Already 32-bit; returns self |
| `get64BitArchVariant()` | No 64-bit variant; falls into "no 64-bit arch" path |
| `getBigEndianArchVariant()` | `cpu0el` → `cpu0` |
| `getLittleEndianArchVariant()` | `cpu0` → `cpu0el` |
| `isLittleEndian()` | `cpu0el` listed as little-endian |
| `getDefaultExceptionHandling()` | No EH (same as csky/arc) |

**Why:** Every architecture that appears in the `ArchType` enum must be handled in each
of these functions or the code will either assert-fail, call `llvm_unreachable`, or
silently produce wrong results. The pattern mirrors the neighboring `csky` entries.

---

### `llvm/lib/TargetParser/TargetDataLayout.cpp`

**What:** Added two cases in `Triple::computeDataLayout()`:

```
cpu0  (BE): "E-m:m-p:32:32-i8:8:32-i16:16:32-i32:32:32-i64:64:64-
             f32:32:32-f64:64:64-v64:64:64-v128:128:128-a:0:64-n32-S64"

cpu0el (LE): same string but leading "e" (little-endian)
```

**Why:** The data layout string encodes ABI alignment and size rules for every IR type.
LLVM IR modules targeting Cpu0 embed this string and tools that read them (optimizer,
linker, etc.) use it to compute struct layouts, load/store widths, and alignment
padding. The layout mirrors MIPS O32 since Cpu0 is MIPS-derived.

---

### `llvm/include/llvm/BinaryFormat/ELF.h`

**What:**
- Added `EM_CPU0 = 999` to the `e_machine` enum.
- Added an anonymous enum block that `#include`s `ELFRelocs/Cpu0.def` to expose all 24
  relocation constants (e.g. `R_CPU0_32`, `R_CPU0_HI16`, `R_CPU0_CALL16`, …).

**Why:** `EM_CPU0` is the value written into the ELF header `e_machine` field of every
object file produced by the Cpu0 backend. Using 999 avoids conflicts with all currently
assigned SYSV/Linux machine IDs (the highest assigned value is EM_LOONGARCH = 258). The
relocation enum lets any LLVM tool refer to relocation types by name instead of raw
integer.

> **Note:** 999 is non-standard and chosen solely for this tutorial. If a real
> architecture is ever assigned a conflicting ID the value will need to change.

---

### `llvm/include/llvm/BinaryFormat/ELFRelocs/Cpu0.def` *(new file)*

**What:** Defines 24 ELF relocation types using the `ELF_RELOC(name, value)` macro
pattern shared by all architectures in this directory.

**Why:** This file is `#include`d in three places — `ELF.h` (enum declaration),
`ELF.cpp` (name lookup), and future linker/MC code — so the single source of truth
lives here. The 24 relocation types cover the full set needed by the Cpu0 tutorial,
modelled after MIPS relocation codes.

---

### `llvm/lib/Object/ELF.cpp`

**What:** Added a `case ELF::EM_CPU0:` block in `getELFRelocationTypeName()` that
`#include`s `Cpu0.def` inside a `switch (Type)` to map relocation integers to their
string names.

**Why:** Tools such as `llvm-readobj` and `llvm-objdump` call this function to print
human-readable relocation names. Without this entry, all Cpu0 relocations would be
printed as `Unknown`.

---

### `llvm/include/llvm/Object/ELFObjectFile.h`

**What:** Added two cases keyed on `ELF::EM_CPU0`:

1. `getFileFormatName()` — returns `"elf32-cpu0"` (BE) or `"elf32-cpu0el"` (LE).
2. `getArch()` — returns `Triple::cpu0` or `Triple::cpu0el` based on endianness; only
   `ELFCLASS32` is valid (64-bit triggers `report_fatal_error`).

**Why:** These methods are called when LLVM opens an existing ELF binary to determine
what target it belongs to. They are the bridge from the binary `e_machine` field back
to the `Triple::ArchType` enum, completing the round-trip.

---

## How to Test

### 1. Build the affected libraries

```bash
cd <build-dir>
cmake --build . --target LLVMTargetParser LLVMObject -j$(nproc)
```

Both libraries must compile with zero errors and zero warnings before proceeding.

### 2. Verify triple parsing (llvm-config / unit tests)

```bash
# Run the TargetParser unit tests
cmake --build . --target TargetParserTests -j$(nproc)
./unittests/TargetParser/TargetParserTests
```

Manually verify round-trip parsing:

```bash
# Using llc (once a backend stub exists) or a small C++ snippet:
Triple T("cpu0-unknown-linux");
assert(T.getArch() == Triple::cpu0);
assert(T.isLittleEndian() == false);

Triple TLE("cpu0el-unknown-linux");
assert(TLE.getArch() == Triple::cpu0el);
assert(TLE.isLittleEndian() == true);

assert(T.getBigEndianArchVariant().getArch()    == Triple::cpu0);
assert(T.getLittleEndianArchVariant().getArch() == Triple::cpu0el);
```

### 3. Verify data layout strings

```bash
# Once llc has a Cpu0 target registered, run:
llc -march=cpu0   -print-before-all /dev/null 2>&1 | grep datalayout
llc -march=cpu0el -print-before-all /dev/null 2>&1 | grep datalayout
```

Expected: big-endian string starts with `E-`, little-endian starts with `e-`.

### 4. Verify ELF relocation name lookup

```bash
cmake --build . --target llvm-readobj -j$(nproc)

# Produce a Cpu0 ELF (once the assembler exists), then:
llvm-readobj -r <cpu0-object.o>
# All relocation entries should print named types (R_CPU0_32, etc.)
# rather than "Unknown (0x...)".
```

### 5. Verify ELF object format name

```bash
llvm-readobj --file-headers <cpu0-object.o> | grep "Format:"
# Expected: elf32-cpu0  (BE) or elf32-cpu0el (LE)
```

### 6. Full build smoke-test

```bash
cmake --build . -j$(nproc) 2>&1 | grep -E "error:|warning:" | head -20
```

Zero errors required. Warnings in unrelated files are acceptable.
