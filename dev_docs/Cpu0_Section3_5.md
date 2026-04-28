# Cpu0 LLVM Backend — Section 3.5: Subtarget Accessors and Chapter Gate CH3_5

## Purpose

Sections 3.3 and 3.4 completed the functional core of the Chapter 3 backend: the stack
frame is now allocated and released correctly, and the register allocator can spill
callee-saved registers. Section 3.5 adds two small utility methods that subsequent
chapters will rely on frequently, and advances the chapter gate from `CH3_2` to `CH3_5`
to unlock the additional conditional-compilation blocks that correspond to this
completed work.

**Section 3.5 goal**:
1. Add `getCpu0Subtarget()` to `Cpu0PassConfig` to eliminate repetitive type-cast chains
   at every call site that needs the subtarget.
2. Add `isLittleEndian()` to `Cpu0TargetMachine` to give callers a single clean query
   for endianness without reaching into `DefaultSubtarget` directly.
3. Advance `Cpu0SetChapter.h` from `CH3_2` to `CH3_5`.

---

## Files Modified

| File | Change | Purpose |
|---|---|---|
| `Cpu0TargetMachine.cpp` | modified | Add `getCpu0Subtarget()` helper method to `Cpu0PassConfig` |
| `Cpu0TargetMachine.h` | modified | Add `isLittleEndian()` accessor to `Cpu0TargetMachine`; minor comment annotation |
| `Cpu0SetChapter.h` | modified | Advance chapter gate from `CH3_2` to `CH3_5` |

---

## Background: The Chapter Gate System

The Cpu0 tutorial uses a compile-time chapter gate (`CH` macro in `Cpu0SetChapter.h`) to
enable code incrementally as each section is completed. Source files use `#if CH >= CH3_3`
guards to include feature code only when the corresponding chapter work is done. This
allows a single source tree to represent any chapter's state by changing one line.

At the end of Section 3.2 the gate was `CH3_2`. Advancing it to `CH3_5` means all
`#if CH >= CH3_3`, `#if CH >= CH3_4`, and `#if CH >= CH3_5` blocks are now active.

```cpp
// Cpu0SetChapter.h — before
#define CH       CH3_2

// Cpu0SetChapter.h — after
#define CH       CH3_5
```

---

## Implementation

### `getCpu0Subtarget()` in `Cpu0PassConfig`

`Cpu0PassConfig` is the inner class (defined inside `Cpu0TargetMachine.cpp`) that
schedules backend passes. Its parent class `TargetPassConfig` provides a
`getTM<T>()` accessor that downcasts the `TargetMachine` to a specific type. To go
one step further and get the `Cpu0Subtarget`, callers previously had to write:

```cpp
// Before: verbose chain at every call site
const Cpu0Subtarget &ST =
    *static_cast<const Cpu0TargetMachine &>(TM).getSubtargetImpl();
```

The new helper collapses this:

```cpp
const Cpu0Subtarget &getCpu0Subtarget() const {
    return *getCpu0TargetMachine().getSubtargetImpl();
}
```

Where `getCpu0TargetMachine()` already exists in `Cpu0PassConfig`:

```cpp
Cpu0TargetMachine &getCpu0TargetMachine() const {
    return getTM<Cpu0TargetMachine>();
}
```

Starting from Chapter 4, pass configuration methods (`addInstSelector`,
`addPreRegAlloc`, etc.) regularly inspect subtarget features to decide which passes
to schedule. Having a single `getCpu0Subtarget()` call per method keeps those
implementations readable.

### `isLittleEndian()` in `Cpu0TargetMachine`

The `Cpu0TargetMachine` class holds a `DefaultSubtarget` member of type
`Cpu0Subtarget`. The subtarget already provides `isLittle()` (set from the target
triple during construction). The new accessor just delegates:

```cpp
bool isLittleEndian() const { return DefaultSubtarget.isLittle(); }
```

This is added to `Cpu0TargetMachine` (the base class shared by `Cpu0ebTargetMachine`
and `Cpu0elTargetMachine`) so that any variant of the target can be queried with the
same call.

Without this accessor, call sites that need to branch on endianness must either
reach directly into `DefaultSubtarget` (breaking encapsulation) or call the more
verbose `getSubtargetImpl()->isLittle()`. Chapter 6 (global variables) and later
chapters use this query to decide relocation and data layout details.

---

## Why These Are in Section 3.5 Rather Than Earlier

Both additions are **pure API convenience**, not functional changes. They do not
alter any generated code and they do not fix any outstanding error. They are placed
at the end of Chapter 3 because:

- Chapter 3 is the last chapter before real ISA work (Chapter 4) begins.
- Adding them now establishes the call patterns that the tutorial's Chapter 4+ code
  will use, so readers can recognize the pattern when they encounter it.
- There is no ordering dependency: they could have been added at any point in Chapter 3
  without changing behavior.

---

## Verification: Chapter Gate Unlocks

After advancing to `CH3_5`, the following conditional blocks become active (examples
from the Cpu0 source tree):

```cpp
// Active from CH3_3 onward: detailed frame info emission in AsmPrinter
#if CH >= CH3_3
  emitFrameDirectives();
#endif

// Active from CH3_5 onward: endianness-aware MC code emission paths
#if CH >= CH3_5
  if (TM.isLittleEndian()) { ... }
#endif
```

These guards exist in files like `Cpu0AsmPrinter.cpp` and `Cpu0MCInstLower.cpp`.
After bumping the gate, recompiling `LLVMCpu0CodeGen` activates them automatically.

---

## How to Test

```bash
# 1. Verify the chapter gate setting
grep 'define CH' llvm/lib/Target/Cpu0/Cpu0SetChapter.h
# Expected: #define CH       CH3_5

# 2. Rebuild
cmake --build build/ --target LLVMCpu0CodeGen -- -j$(nproc)
cmake --build build/ --target llc -- -j$(nproc)

# 3. Run the full Chapter 3 test suite (ch3.bc → Cpu0 assembly)
build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm \
    /tmp/ch3.bc -o /tmp/ch3.s
echo "Exit: $?"   # Expected: 0

# 4. Test little-endian target (cpu0el)
build/bin/llc -march=cpu0el -relocation-model=pic -filetype=asm \
    /tmp/ch3.bc -o /tmp/ch3el.s
echo "Exit: $?"   # Expected: 0

# 5. Confirm isLittleEndian() difference between targets
# (Chapter 6+ will exercise this path more directly.)
```

---

## State After Section 3.5 (End of Chapter 3)

| Capability | Status |
|---|---|
| `getCpu0Subtarget()` helper in `Cpu0PassConfig` | ✅ |
| `isLittleEndian()` accessor in `Cpu0TargetMachine` | ✅ |
| Chapter gate advanced to `CH3_5` | ✅ |
| `emitPrologue` / `emitEpilogue` with chunked `ADDiu` | ✅ (Section 3.3) |
| `storeRegToStackSlot` / `loadRegFromStackSlot` | ✅ (Section 3.4) |
| `llc` exits 0 for functions with local variables | ✅ |
| `llc` exits 0 for little-endian target (`cpu0el`) | ✅ |
| Full ISA (arithmetic, Load/Store, branches, calls) | ❌ Chapter 4 |
| Object file emission (`-filetype=obj`) | ❌ Chapter 5 |
| Global variable support | ❌ Chapter 6 |

### Chapter 3 in One Sentence

Chapter 3 took the Cpu0 backend from an empty class skeleton (3.1) through crash fixes
(3.2), stack frame implementation (3.3), callee-saved register spill/restore (3.4), and
subtarget utility methods (3.5) — completing a fully functional backend skeleton that can
compile a simple C function end-to-end to Cpu0 assembly.
