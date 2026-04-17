# Cpu0 LLVM Backend — Section 2.4.3: Target Registration

## Purpose

This step wires the Cpu0 backend into LLVM's target registration infrastructure
so that LLVM tools can discover and instantiate the Cpu0 target at runtime.
After this section, `llc -march=cpu0` (or `llc -march=cpu0el`) resolves to
a concrete target machine, and all three LLVM component libraries
(`LLVMCpu0Info`, `LLVMCpu0Desc`, `LLVMCpu0CodeGen`) build cleanly.

This is a Chapter 2 skeleton: the three `LLVMInitialize*` functions are wired
up correctly but most of their bodies are empty stubs. MC components
(instruction printer, asm info, code emitter) are filled in starting at
Chapter 3.2.

---

## Files Modified

### New files

| File | Purpose |
|------|---------|
| `llvm/lib/Target/Cpu0/CMakeLists.txt` | Top-level build: tablegen invocations + `add_llvm_target(Cpu0CodeGen)` |
| `llvm/lib/Target/Cpu0/Cpu0.h` | Main backend header; forward-declares `Cpu0TargetMachine` and `FunctionPass` |
| `llvm/lib/Target/Cpu0/Cpu0TargetMachine.h` | Declares `Cpu0TargetMachine` (base), `Cpu0ebTargetMachine` (BE), `Cpu0elTargetMachine` (LE) |
| `llvm/lib/Target/Cpu0/Cpu0TargetMachine.cpp` | Implements constructors and `LLVMInitializeCpu0Target()` |
| `llvm/lib/Target/Cpu0/TargetInfo/CMakeLists.txt` | Builds `LLVMCpu0Info` component library |
| `llvm/lib/Target/Cpu0/TargetInfo/Cpu0TargetInfo.h` | Declares `getTheCpu0Target()` and `getTheCpu0elTarget()` singletons |
| `llvm/lib/Target/Cpu0/TargetInfo/Cpu0TargetInfo.cpp` | Implements singletons and `LLVMInitializeCpu0TargetInfo()` |
| `llvm/lib/Target/Cpu0/MCTargetDesc/CMakeLists.txt` | Builds `LLVMCpu0Desc` component library |
| `llvm/lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h` | Includes tablegen-generated register/instruction/subtarget enum `.inc` files |
| `llvm/lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp` | Empty `LLVMInitializeCpu0TargetMC()` stub |

### Modified files

| File | Change |
|------|--------|
| `llvm/CMakeLists.txt` | Added `Cpu0` to `LLVM_ALL_EXPERIMENTAL_TARGETS` |
| `llvm/lib/Target/Cpu0/Cpu0.td` | Added `defm : RemapAllTargetPseudoPointerOperands<CPURegs>` |

---

## Why Each File Was Modified

### `llvm/CMakeLists.txt`
LLVM only builds a target if it appears in `LLVM_ALL_TARGETS` (production) or
`LLVM_ALL_EXPERIMENTAL_TARGETS` (experimental). Adding `Cpu0` here allows the
target to be included via `-DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=Cpu0` at cmake
configure time.

### `llvm/lib/Target/Cpu0/Cpu0.td`
Modern LLVM tablegen requires every target to remap the `ptr_rc`
`PointerLikeRegClass` used in standard pseudo-instructions such as
`PATCHABLE_EVENT_CALL` and `LOAD_STACK_GUARD`. Without this line, tablegen
errors out on all four affected pseudo-instructions when generating
`Cpu0GenInstrInfo.inc`. The pattern is identical to ARC, CSKY, and M68k.

### `TargetInfo/Cpu0TargetInfo.cpp`
Defines the two global `Target` singletons (`TheCpu0Target`, `TheCpu0elTarget`)
and registers them with `TargetRegistry` via `RegisterTarget<Triple::cpu0>` and
`RegisterTarget<Triple::cpu0el>`. This is the entry point that `llc --version`
and `llc -march=cpu0` use to discover the target.

### `MCTargetDesc/Cpu0MCTargetDesc.cpp`
The stub `LLVMInitializeCpu0TargetMC()` is the hook point for registering MC
components (asm info, instruction info, register info, subtarget info, inst
printer) in later chapters. The include of the generated `.inc` files through
`Cpu0MCTargetDesc.h` makes register/instruction/subtarget enumeration available
to any consumer of this header.

### `Cpu0TargetMachine.cpp`
`LLVMInitializeCpu0Target()` registers both `Cpu0ebTargetMachine` (big-endian)
and `Cpu0elTargetMachine` (little-endian) with `TargetRegistry`. The
constructors call `TT.computeDataLayout()` which resolves to the MIPS-style
layout strings already placed in `TargetDataLayout.cpp` during Section 2.4.1.

### `CMakeLists.txt` (top-level Cpu0)
Drives tablegen to generate four `.inc` files from `Cpu0Other.td`, then builds
`Cpu0TargetMachine.cpp` into `LLVMCpu0CodeGen` with its three subdirectory
dependencies (`Cpu0Desc`, `Cpu0Info`, CodeGen, SelectionDAG, etc.).

---

## How to Test

### 1. Configure the build with Cpu0 enabled

```bash
cmake -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=Cpu0 <build-dir>
```

If the build directory already exists (as in this repo), re-run cmake from the
project root:

```bash
cmake -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=Cpu0 build
# Expected: "-- Targeting Cpu0" in the configure output
```

### 2. Build the three Cpu0 libraries

```bash
cmake --build build --target LLVMCpu0Info LLVMCpu0Desc LLVMCpu0CodeGen
```

Expected output (zero errors):
```
[N/452] Building Cpu0GenCallingConv.inc...
[N/452] Building Cpu0GenInstrInfo.inc...
[N/452] Building Cpu0GenRegisterInfo.inc...
[N/452] Building Cpu0GenSubtargetInfo.inc...
[N/452] Linking CXX static library lib/libLLVMCpu0Info.a
[N/452] Linking CXX static library lib/libLLVMCpu0Desc.a
[N/452] Linking CXX static library lib/libLLVMCpu0CodeGen.a
```

### 3. Verify target registration (optional, requires llc)

```bash
cmake --build build --target llc
build/bin/llc --version | grep cpu0
```

Expected:
```
    cpu0       - CPU0 (32-bit big endian)
    cpu0el     - CPU0 (32-bit little endian)
```

### 4. Smoke-check the tablegen output

```bash
grep -r 'LLVMInitializeCpu0TargetInfo' llvm/lib/Target/Cpu0/TargetInfo/Cpu0TargetInfo.cpp
grep -r 'LLVMInitializeCpu0TargetMC'   llvm/lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
grep -r 'LLVMInitializeCpu0Target\b'   llvm/lib/Target/Cpu0/Cpu0TargetMachine.cpp
grep -r 'RegisterTargetMachine<Cpu0ebTargetMachine>' llvm/lib/Target/Cpu0/Cpu0TargetMachine.cpp
grep -r 'RegisterTargetMachine<Cpu0elTargetMachine>' llvm/lib/Target/Cpu0/Cpu0TargetMachine.cpp
```

All five greps should find exactly one match each.

---

## Key Design Decisions

- **Singleton accessor pattern** (`getTheCpu0Target()` / `getTheCpu0elTarget()`)
  instead of global `TheCpu0Target` variables — matches modern LLVM convention
  (ARC, CSKY) and avoids static-initialization order issues.

- **`getSubtargetImpl()` returns `nullptr`** — `Cpu0Subtarget` does not exist
  yet at Chapter 2. The method is pure virtual in `LLVMTargetMachine` so it must
  be overridden; returning `nullptr` is safe as long as no code is actually
  compiled for Cpu0 at this stage.

- **`TT.computeDataLayout()`** — the data layout strings for `Triple::cpu0`
  (big-endian) and `Triple::cpu0el` (little-endian) were added to
  `TargetDataLayout.cpp` in Section 2.4.1, so the target machine constructor
  can simply call this method rather than hard-coding the string.

- **`RemapAllTargetPseudoPointerOperands<CPURegs>`** — required in modern LLVM
  to map `ptr_rc` (used by several standard pseudo-instructions in `Target.td`)
  to a concrete Cpu0 register class. Without it, tablegen fails to generate
  `Cpu0GenInstrInfo.inc`.
