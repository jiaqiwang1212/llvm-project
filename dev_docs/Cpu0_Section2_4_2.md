# Cpu0 Backend — Section 2.4.2: Creating the Initial Cpu0 .td Files

## Purpose

This step creates the TableGen description files (`.td` files) that define the Cpu0
target's register set, instruction formats, instructions, scheduling model, and calling
conventions. LLVM's `llvm-tblgen` tool translates these files into C++ header files
(`.inc` files) that are then compiled into the backend. No hand-written C++ code can
describe registers or instructions — that information must come from `.td` files.

Specifically, these files establish:

1. The complete Cpu0 register file (16 general-purpose registers, 2 co-processor registers).
2. The three instruction encoding formats (A, L, J) used by every Cpu0 instruction.
3. A minimal initial instruction set (LD, ST, ADDiu, JR, RET, NOP) sufficient to compile
   and link trivial programs once the full backend skeleton is in place.
4. The instruction scheduling model (itineraries) used by the scheduler.
5. The calling convention and callee-saved register set for the O32 ABI.
6. SubtargetFeature definitions that gate chapter-by-chapter tutorial additions.

---

## Files Modified

All files are new; none of the files below existed before this step.

---

### `llvm/lib/Target/Cpu0/Cpu0RegisterInfo.td` *(new file)*

**What:** Defines the Cpu0 register file.

- Base register class `Cpu0Reg` (inherits LLVM's `Register`, sets `HWEncoding` and
  `Namespace = "Cpu0"`).
- Sub-classes `Cpu0GPRReg` (general-purpose) and `Cpu0C0Reg` (co-processor 0).
- 16 GPRs in the `Cpu0` namespace:

| Register | Encoding | Role |
|----------|----------|------|
| ZERO | 0 | Hard-wired zero |
| AT | 1 | Assembler temporary |
| V0–V1 | 2–3 | Return values / arguments |
| A0–A1 | 4–5 | Arguments |
| T9 | 6 | Indirect-call target |
| T0–T1 | 7–8 | Temporaries (not callee-saved) |
| S0–S1 | 9–10 | Callee-saved |
| GP | 11 | Global pointer |
| FP | 12 | Frame pointer |
| SP | 13 | Stack pointer |
| LR | 14 | Link register (return address) |
| SW | 15 | Status/condition-codes word |

- 2 co-processor 0 registers: `PC` (enc 0) and `EPC` (enc 1).
- Register classes `CPURegs` (all 16 GPRs), `SR` (SW only), `C0Regs` (PC + EPC).

**Why:** Every LLVM backend must declare its physical registers in `.td`. TableGen
generates `Cpu0GenRegisterInfo.inc` from this file, which provides the register
numbering, DWARF mappings, and class membership tables used throughout the backend.

---

### `llvm/lib/Target/Cpu0/Cpu0RegisterInfoGPROutForOther.td` *(new file)*

**What:** Defines `GPROut` as `CPURegs` minus `SW`:

```
def GPROut : RegisterClass<"Cpu0", [i32], 32, (add (sub CPURegs, SW))>;
```

**Why:** The SW (status word) register holds condition-code bits set by compare
instructions. It must not be allocated as a general output register — allocating SW
as an output would silently overwrite the condition codes. `GPROut` is used as the
output class for all arithmetic and load instructions, preventing this.

This file is kept separate because the AsmParser variant of the register info includes
a different version that also exposes SW to the parser (for disassembly purposes).

---

### `llvm/lib/Target/Cpu0/Cpu0Schedule.td` *(new file)*

**What:** Defines the Cpu0 instruction scheduling model:

- Two functional units: `ALU` and `IMULDIV`.
- Seven itinerary classes: `IIAlu`, `II_CLO`, `II_CLZ`, `IILoad`, `IIStore`,
  `IIBranch`, `IIPseudo`.
- `Cpu0GenericItineraries` — a `ProcessorItineraries` record mapping each itinerary
  class to an execution stage (e.g. loads take 3 ALU cycles; all others take 1).

**Why:** The `Processor` records in `Cpu0.td` reference `Cpu0GenericItineraries`. TableGen
generates `Cpu0GenSubtargetInfo.inc` from this data, which the LLVM scheduler uses to
model instruction latencies and functional-unit occupancy.

---

### `llvm/lib/Target/Cpu0/Cpu0InstrFormats.td` *(new file)*

**What:** Defines the base instruction classes for the three Cpu0 encoding formats:

| Class | Format | Bit layout |
|-------|--------|------------|
| `Cpu0Inst` | Base (32-bit) | `Inst{31-24}` = opcode |
| `FA` | A-type (3-reg) | `opcode[8] | ra[4] | rb[4] | rc[4] | shamt[12]` |
| `FL` | L-type (reg+imm) | `opcode[8] | ra[4] | rb[4] | imm16[16]` |
| `FJ` | J-type (jump) | `opcode[8] | addr24[24]` |

Also defines `Format` metadata (`Pseudo`, `FrmA`, `FrmL`, `FrmJ`, `FrmOther`) stored
in the `TSFlags` field of every instruction, which the code emitter uses at runtime
to select the correct binary encoding path.

**Why:** Encoding formats are the foundation of instruction definitions. Every instruction
class in `Cpu0InstrInfo.td` inherits from `FA`, `FL`, or `FJ` and picks up the correct
bit-field layout automatically. Separating formats from instructions keeps the files
manageable as the instruction count grows.

---

### `llvm/lib/Target/Cpu0/Cpu0InstrInfo.td` *(new file)*

**What:** Defines the initial Cpu0 instruction set and selection patterns.

Key sections:

- **SDNode types:** `SDT_Cpu0Ret` and `Cpu0Ret` for the Cpu0-specific return node
  (`Cpu0ISD::Ret`).
- **Chapter predicates:** `Ch3_1` through `Ch_all` — one predicate per tutorial chapter,
  so instructions can be gated to enable only the subset relevant to the current chapter.
- **Overflow predicates:** `EnableOverflow` / `DisableOverflow` — used in later chapters
  for ADD vs. ADDU variants.
- **Operand types:** `simm16` (signed 16-bit immediate), `mem` (base+offset address),
  `immSExt16` (pattern leaf), `addr` (complex pattern for frame-index addresses).
- **Instruction classes:** `ArithLogicI`, `FMem`, `LoadM`, `StoreM`, `JumpFR`, `RetBase`.
- **Instruction definitions:**

| Instruction | Opcode | Description |
|-------------|--------|-------------|
| `LD` | 0x01 | 32-bit aligned load |
| `ST` | 0x02 | 32-bit aligned store |
| `ADDiu` | 0x09 | Add immediate (signed 16-bit, no overflow trap) |
| `JR` | 0x3c | Jump register (indirect branch) |
| `RET` | 0x3c | Return (pseudo, maps to JR LR) |
| `NOP` | 0x00 | No operation |

- **Selection pattern:** `(i32 immSExt16:$in) → ADDiu ZERO, imm:$in` (materialize
  small constants).

**LLVM 20+ adaptations (vs. tutorial source targeting LLVM 12):**
- `SDNPWantParent` removed from the `addr` `ComplexPattern` — this node property was
  removed from LLVM and is no longer defined in `TargetSelectionDAG.td`.
- `LD->getAlignment()` / `SD->getAlignment()` replaced by `LD->getAlign().value()` /
  `SD->getAlign().value()` — `getAlignment()` was removed; `getAlign()` returns an
  `llvm::Align` object whose `.value()` yields the integer.

**Why:** This file is the primary instruction description. TableGen generates
`Cpu0GenDAGISel.inc` (instruction selection), `Cpu0GenInstrInfo.inc` (MC instruction
descriptors), and `Cpu0GenAsmWriter.inc` (assembly printing) from this file.

---

### `llvm/lib/Target/Cpu0/Cpu0CallingConv.td` *(new file)*

**What:**

- `CCIfSubtarget` — a helper `CCIf` predicate that tests a subtarget feature string at
  compile time for use in calling-convention rules.
- `CSR_O32` — the O32 callee-saved register list: `LR`, `FP`, `S1`, `S0` (in that
  order, matching the reference implementation's push order).

**Why:** TableGen generates `Cpu0GenCallingConv.inc` from this file. The frame-lowering
code uses `CSR_O32` to know which registers must be spilled on function entry and
restored on exit.

---

### `llvm/lib/Target/Cpu0/Cpu0.td` *(new file)*

**What:** The main target description file. Defines:

- **SubtargetFeatures:** One `SubtargetFeature` per tutorial chapter (`FeatureChapter3_1`
  through `FeatureChapter12_1`), plus `FeatureChapterAll` (depends on all chapters),
  `FeatureCmp`, `FeatureSlt`, `FeatureCpu032I`, and `FeatureCpu032II`.
- **Includes:** `Cpu0Schedule.td`, `Cpu0InstrInfo.td`, `Cpu0CallingConv.td`.
- **`Cpu0InstrInfo` record** — the `InstrInfo` singleton used by the `Target` def.
- **Processor definitions:** `cpu032I` (uses `FeatureCpu032I` = cmp + all chapters) and
  `cpu032II` (adds `FeatureSlt`).
- **`def Cpu0 : Target`** — registers the target with LLVM's target registry and sets
  `InstructionSet = Cpu0InstrInfo`.

**Why:** The `Target` definition is what `llvm-tblgen` uses as the root when generating
subtarget and processor-specific tables. The `SubtargetFeature` definitions allow the
tutorial to incrementally add instructions chapter by chapter without breaking earlier
chapters.

---

### `llvm/lib/Target/Cpu0/Cpu0Other.td` *(new file)*

**What:** The top-level entry point for `llvm-tblgen`. Includes:

```tablegen
include "llvm/Target/Target.td"           // LLVM built-in base classes
include "Cpu0RegisterInfo.td"
include "Cpu0RegisterInfoGPROutForOther.td"
include "Cpu0.td"                         // which itself includes Schedule/InstrInfo/CallingConv
```

`LLVM_TARGET_DEFINITIONS` in `CMakeLists.txt` will point to this file.

**Why:** TableGen requires a single root file. Using `Cpu0Other.td` as the root (rather
than `Cpu0.td`) keeps the AsmParser-vs-everything-else split clean: the AsmParser
variant uses a different register info that includes SW in the allocatable set.

---

## How to Test

### 1. Verify all .td files exist

```bash
ls llvm/lib/Target/Cpu0/*.td
```

Expected output (8 files):
```
Cpu0.td  Cpu0CallingConv.td  Cpu0InstrFormats.td  Cpu0InstrInfo.td
Cpu0Other.td  Cpu0RegisterInfo.td  Cpu0RegisterInfoGPROutForOther.td  Cpu0Schedule.td
```

### 2. Parse with llvm-tblgen

```bash
# Build tblgen if not already built
cmake --build <build-dir> --target llvm-tblgen

# Run tblgen on the top-level entry point
<build-dir>/bin/llvm-tblgen \
  -I llvm/include \
  -I llvm/lib/Target \
  -I llvm/lib/Target/Cpu0 \
  llvm/lib/Target/Cpu0/Cpu0Other.td
```

Expected: exit code 0, no `error:` lines in stderr.

### 3. Verify specific definitions are present

```bash
# Register definitions
grep "def ZERO"   llvm/lib/Target/Cpu0/Cpu0RegisterInfo.td
grep "def CPURegs" llvm/lib/Target/Cpu0/Cpu0RegisterInfo.td
grep "def GPROut"  llvm/lib/Target/Cpu0/Cpu0RegisterInfoGPROutForOther.td

# Instruction formats
grep "class Cpu0Inst" llvm/lib/Target/Cpu0/Cpu0InstrFormats.td
grep "class FJ"       llvm/lib/Target/Cpu0/Cpu0InstrFormats.td

# Instructions
grep "def ADDiu" llvm/lib/Target/Cpu0/Cpu0InstrInfo.td
grep "def RET"   llvm/lib/Target/Cpu0/Cpu0InstrInfo.td

# Processor and target
grep "def Cpu0 : Target"    llvm/lib/Target/Cpu0/Cpu0.td
grep "def CSR_O32"          llvm/lib/Target/Cpu0/Cpu0CallingConv.td
grep "Cpu0GenericItineraries" llvm/lib/Target/Cpu0/Cpu0Schedule.td
```

All greps must find exactly one match.

### 4. Generate register info (smoke test tablegen output)

```bash
<build-dir>/bin/llvm-tblgen \
  -I llvm/include \
  -I llvm/lib/Target \
  -I llvm/lib/Target/Cpu0 \
  -gen-register-info \
  llvm/lib/Target/Cpu0/Cpu0Other.td 2>&1 | head -30
```

Expected: C++ output beginning with `// ...` auto-generated header; no `error:` lines.

### 5. Generate instruction info

```bash
<build-dir>/bin/llvm-tblgen \
  -I llvm/include \
  -I llvm/lib/Target \
  -I llvm/lib/Target/Cpu0 \
  -gen-instr-info \
  llvm/lib/Target/Cpu0/Cpu0Other.td 2>&1 | grep -c "Cpu0"
```

Expected: a non-zero count of lines containing "Cpu0" (confirms instructions were
recognized and emitted into the table).

### 6. Full library build (once CMakeLists.txt and .cpp stubs are added in 2.4.3–2.4.4)

```bash
cmake --build <build-dir> --target Cpu0CodeGen -j$(nproc)
```

This step is covered in section 2.4.4 and will not succeed until the C++ stub files
from section 2.4.3 are present.
