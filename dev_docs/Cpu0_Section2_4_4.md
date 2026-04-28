# Cpu0 LLVM Backend — Section 2.4.4: Build Libraries and .td Files

## Purpose

This step verifies that everything from Sections 2.4.1–2.4.3 fits together by
actually building the Cpu0 backend libraries. At the end of this section, three
static libraries exist in the build output:

| Library | Contents |
|---------|----------|
| `libLLVMCpu0Info.a` | Target registration singletons (`getTheCpu0Target`, `getTheCpu0elTarget`) |
| `libLLVMCpu0Desc.a` | MC layer stub (`LLVMInitializeCpu0TargetMC`); owns the generated `.inc` enums |
| `libLLVMCpu0CodeGen.a` | Target machine skeleton (`Cpu0ebTargetMachine`, `Cpu0elTargetMachine`) |

TableGen also generates four `.inc` files that the C++ source includes at
compile time:

| Generated file | Generator flag | Contents |
|----------------|----------------|----------|
| `Cpu0GenCallingConv.inc` | `-gen-callingconv` | `CSR_O32` callee-saved register list |
| `Cpu0GenInstrInfo.inc` | `-gen-instr-info` | Instruction descriptor table |
| `Cpu0GenRegisterInfo.inc` | `-gen-register-info` | Register numbering, DWARF mappings, class tables |
| `Cpu0GenSubtargetInfo.inc` | `-gen-subtarget` | Processor itineraries and feature bits |

No new source files are introduced in this section. The sole deliverable is a
successful build.

---

## Build System Wiring

### How `CMakeLists.txt` drives TableGen

`llvm/lib/Target/Cpu0/CMakeLists.txt` contains:

```cmake
set(LLVM_TARGET_DEFINITIONS Cpu0Other.td)

tablegen(LLVM Cpu0GenCallingConv.inc  -gen-callingconv)
tablegen(LLVM Cpu0GenInstrInfo.inc    -gen-instr-info)
tablegen(LLVM Cpu0GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM Cpu0GenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(Cpu0CommonTableGen)
```

`LLVM_TARGET_DEFINITIONS` tells the LLVM CMake macros that `llvm-tblgen` should
be invoked with `Cpu0Other.td` as its root file. Each `tablegen()` call adds a
build rule that produces one `.inc` file. `add_public_tablegen_target` creates a
synthetic CMake target (`Cpu0CommonTableGen`) that all three libraries depend on,
ensuring TableGen runs before any C++ compilation starts.

`Cpu0Other.td` is the entry point because it includes the register info variants
in the correct order:

```tablegen
include "llvm/Target/Target.td"
include "Cpu0RegisterInfo.td"
include "Cpu0RegisterInfoGPROutForOther.td"
include "Cpu0.td"                // pulls in Schedule, InstrInfo, CallingConv
```

### Library dependency chain

```
LLVMCpu0CodeGen
  ├── LLVMCpu0Desc
  │     └── LLVMCpu0Info
  └── Cpu0CommonTableGen  (tablegen .inc files)
```

`LLVMCpu0CodeGen` links `LLVMCpu0Desc` and `LLVMCpu0Info` as components (via the
`LINK_COMPONENTS` block in `add_llvm_target`), so building `LLVMCpu0CodeGen`
transitively builds the other two.

---

## Prerequisites

Before running the build commands below, confirm that:

1. The build directory is configured with Cpu0 enabled:
   ```
   grep Cpu0 build/CMakeCache.txt
   # Expected: LLVM_EXPERIMENTAL_TARGETS_TO_BUILD:STRING=Cpu0
   ```

2. All source files from Sections 2.4.2 and 2.4.3 exist:
   ```bash
   ls llvm/lib/Target/Cpu0/*.td llvm/lib/Target/Cpu0/*.h \
      llvm/lib/Target/Cpu0/*.cpp \
      llvm/lib/Target/Cpu0/TargetInfo/ \
      llvm/lib/Target/Cpu0/MCTargetDesc/
   ```

---

## How to Build

### Build all three Cpu0 libraries

```bash
cmake --build build --target LLVMCpu0Info LLVMCpu0Desc LLVMCpu0CodeGen
```

Because `LLVMCpu0CodeGen` depends on the other two, you can also just build the
top-level target:

```bash
cmake --build build --target LLVMCpu0CodeGen
```

### Parallel build (faster)

```bash
cmake --build build --target LLVMCpu0CodeGen -- -j$(sysctl -n hw.ncpu)
```

---

## How to Test

### 1. Verify libraries exist and are non-empty

```bash
ls -lh build/lib/libLLVMCpu0Info.a \
        build/lib/libLLVMCpu0Desc.a \
        build/lib/libLLVMCpu0CodeGen.a
```

All three files must exist and have size > 0.

### 2. Verify TableGen generated all four .inc files

```bash
ls -lh build/lib/Target/Cpu0/Cpu0Gen*.inc
```

Expected (four files, all non-empty):
```
Cpu0GenCallingConv.inc
Cpu0GenInstrInfo.inc
Cpu0GenRegisterInfo.inc
Cpu0GenSubtargetInfo.inc
```

### 3. Inspect a generated .inc file

```bash
head -5 build/lib/Target/Cpu0/Cpu0GenRegisterInfo.inc
```

Expected: auto-generated C++ header comment followed by `#ifdef` guards — no
`error:` lines.

### 4. Verify symbols are present in the libraries

```bash
nm build/lib/libLLVMCpu0Info.a    | grep LLVMInitializeCpu0TargetInfo
nm build/lib/libLLVMCpu0Desc.a    | grep LLVMInitializeCpu0TargetMC
nm build/lib/libLLVMCpu0CodeGen.a | grep LLVMInitializeCpu0Target
```

Each `nm` command must print at least one line.

### 5. Verify target registration (requires llc)

```bash
cmake --build build --target llc
build/bin/llc --version | grep -i cpu0
```

Expected:
```
    cpu0       - CPU0 (32-bit big endian)
    cpu0el     - CPU0 (32-bit little endian)
```

---

## Troubleshooting

### `ninja: error: unknown target 'Cpu0CodeGen'`

The CMake library target uses the `LLVM`-prefixed name:
```bash
cmake --build build --target LLVMCpu0CodeGen   # correct
cmake --build build --target Cpu0CodeGen        # wrong
```

### `tablegen: error: Undefined or non-concrete class/def 'ptr_rc'`

The `Cpu0.td` file is missing the `RemapAllTargetPseudoPointerOperands` line.
Modern LLVM's `Target.td` uses `ptr_rc` in several pseudo-instruction
definitions; each target must remap it to a concrete register class. Add to
`Cpu0.td`:

```tablegen
defm : RemapAllTargetPseudoPointerOperands<CPURegs>;
```

### `error: 'getAlignment' is not a member of ...`

This indicates a C++ file uses a LLVM 12-era API. Replace
`LD->getAlignment()` / `SD->getAlignment()` with `LD->getAlign().value()` /
`SD->getAlign().value()` in any `.cpp` files that access memory alignment.

---

## Key Design Decisions

- **`Cpu0Other.td` as TableGen root** — using a thin wrapper file rather than
  `Cpu0.td` directly keeps the AsmParser path cleanly separated. The AsmParser
  variant includes a different register info file that exposes SW to the parser;
  `Cpu0Other.td` is the non-parser path.

- **Four `.inc` files, not one** — each TableGen generator (`-gen-callingconv`,
  `-gen-instr-info`, etc.) produces a single-purpose header. Separate files avoid
  macro-guard conflicts and allow each consumer to include only what it needs.

- **`LLVMCpu0CodeGen` depends on `LLVMCpu0Desc` and `LLVMCpu0Info`** — the
  three-library split matches LLVM's layering convention: `Info` is link-time-only
  registration; `Desc` is the MC layer (binary format, relocation); `CodeGen` is
  the SelectionDAG/instruction-selection layer. Later chapters fill in `Desc` and
  `CodeGen` iteratively without changing the build structure.
