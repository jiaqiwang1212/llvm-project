<!-- Generated: 2026-05-07 -->

# Bitcode/Reader — Bitcode Deserialization

## Purpose

Deserializes LLVM bitcode (.bc files) back into IR modules and functions. Reads low-level bitstream records and reconstructs type systems, constants, functions, instructions, metadata, and debug information. Supports lazy deserialization for efficient link-time optimization.

## Key Files

| File | Description |
|------|-------------|
| `BitcodeReader.cpp` | Main bitcode deserialization engine (parses all bitcode records) |
| `MetadataLoader.cpp` | Metadata record deserialization and reconstruction |
| `ValueList.cpp` | Mapping of bitcode value IDs to IR values |
| `BitcodeAnalyzer.cpp` | Analyzes bitcode structure (statistics and debugging) |
| `BitReader.cpp` | C API for bitcode reading |
| `MetadataLoader.h` | Metadata loader interface |
| `ValueList.h` | Value list interface |

## Subdirectories

None. This is a leaf directory under `llvm/lib/Bitcode/`.

## For AI Agents

### Working In This Directory

1. **Lazy loading**: Bitcode reader supports lazy function materialization. Functions are not fully deserialized until needed, enabling efficient LTO. Understand the lazy loading mechanism before modifying.

2. **Value IDs**: Bitcode uses numeric value IDs (not names). The `ValueList` maps these IDs to IR `Value` objects. Maintain this invariant.

3. **Type reconstruction**: Types are reconstructed from type records. The type system must be built correctly or subsequent instructions will be malformed.

4. **Metadata is separate**: Metadata is loaded after the main IR. The `MetadataLoader` handles this separately to avoid circular dependencies.

5. **Error handling**: Corrupted bitcode can cause crashes if not handled carefully. Validate records before processing.

6. **Backward compatibility**: Old bitcode formats must be supported. Use version information to handle format changes gracefully.

7. **Memory efficiency**: Large modules may have thousands of functions. Lazy loading avoids materializing unused functions, saving memory.

### Common Patterns

**Deserializing a record:**
```cpp
// In BitcodeReader.cpp
case bitc::MODULE_CODE_FUNCTION: {
  if (Record.size() < 4)
    return error("Invalid function record");
  auto [Type, CallingConv, Attrs, ...] = parseRecord(Record);
  Function *F = Function::Create(FTy, LinkageType, Name, M);
  // ...
}
```

**Handling value references:**
```cpp
Value *V = getValueFwdRef(ValueID);  // Gets value or forward reference
```

**Lazy materialization:**
```cpp
// In BitcodeReader.cpp
if (Lazy) {
  F->setParamAttr(...);
  F->setMaterializable(true);
  // Don't deserialize body yet
} else {
  materializeFunction(F);  // Deserialize full function
}
```

## Dependencies

### Internal

- **llvm/lib/IR/** — IR objects being constructed
- **llvm/lib/Bitstream/Reader/** — Low-level bitstream reading (see ../Reader/AGENTS.md)
- **llvm/lib/Support/** — String and data utilities
- **llvm/include/llvm/Bitcode/BitcodeReader.h** — Public reader header

### External

None.

### Dependents

- **llvm/tools/llvm-dis** — Disassembles bitcode to text
- **Link-time optimization** — Uses bitcode reader for lazy loading
- **Any tool that reads .bc files**

## Notes for Developers

- **Record numbers**: Bitcode records are numbered in `llvm/include/llvm/Bitcode/LLVMBitCodes.h`. Consult this file when adding/modifying records.
- **Forward references**: Values can be referenced before they are defined. Use `getValueFwdRef()` to create forward references.
- **Type resolution**: Some records depend on types that haven't been loaded yet. Implement two-pass or deferred loading if necessary.
- **Verification**: After deserialization, call `verifyModule()` to catch corrupt bitcode early.
- **Performance**: Bitcode reading is on the critical path for LTO. Profile and optimize hot code paths.
- **Testing**: Add test cases in `llvm/test/Bitcode/` for new record formats.

<!-- MANUAL: -->
