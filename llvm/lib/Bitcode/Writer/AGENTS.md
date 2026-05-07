<!-- Generated: 2026-05-07 -->

# Bitcode/Writer — Bitcode Serialization

## Purpose

Serializes LLVM IR modules into bitcode (.bc files). Walks the IR data structures and emits them as bitstream records, including instructions, types, constants, metadata, and debug information. Produces the inverse of the Reader—the Reader deserializes what the Writer produces.

## Key Files

| File | Description |
|------|-------------|
| `BitcodeWriter.cpp` | Main bitcode serialization engine (emits all bitcode records) |
| `ValueEnumerator.cpp` | Assigns numeric IDs to IR values for bitcode serialization |
| `BitcodeWriterPass.cpp` | Pass interface for writing bitcode as an optimization pass |
| `BitWriter.cpp` | C API for bitcode writing |
| `ValueEnumerator.h` | Value enumerator interface |

## Subdirectories

None. This is a leaf directory under `llvm/lib/Bitcode/`.

## For AI Agents

### Working In This Directory

1. **Value numbering**: The Writer must assign consistent numeric IDs to values. The `ValueEnumerator` handles this. Understand the numbering scheme before modifying.

2. **IR traversal order**: Values must be emitted in a specific order to match the Reader's expectations. Forward references are handled, but order matters for efficiency.

3. **Type emission**: Types must be emitted before values that use them. Maintain this ordering.

4. **Metadata ordering**: Metadata is emitted after the main IR structure. The `MetadataLoader` in Reader expects this order.

5. **Abbreviations**: Bitcode uses custom abbreviations to compress common record patterns. Use abbreviations for frequently-emitted records.

6. **Format compatibility**: The Writer must produce bitcode readable by the corresponding Reader version. Maintain version compatibility.

7. **Performance**: Bitcode writing is fast but can be a bottleneck in LTO. Use efficient bit-packing and abbreviations.

### Common Patterns

**Emitting a record:**
```cpp
// In BitcodeWriter.cpp
SmallVector<uint64_t, 4> Record;
Record.push_back(ValueID(V));
Record.push_back(Instruction::Add);
Stream.EmitRecord(bitc::FUNC_CODE_INST_BINOP, Record);
```

**Using abbreviations:**
```cpp
auto Abbrev = std::make_shared<BitCodeAbbrev>();
Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 6));  // Opcode
Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));   // LHS
Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));   // RHS
unsigned BinOpAbbrev = Stream.EmitAbbrev(Abbrev);
```

**Writing a function:**
```cpp
for (auto &BB : F) {
  for (auto &I : BB) {
    writeInstruction(I);
  }
}
```

## Dependencies

### Internal

- **llvm/lib/IR/** — IR objects being serialized
- **llvm/lib/Bitstream/Reader/** — Low-level bitstream writing (see ../Reader/AGENTS.md)
- **llvm/lib/Support/** — String and data utilities
- **llvm/include/llvm/Bitcode/BitcodeWriter.h** — Public writer header

### External

None.

### Dependents

- **llvm/tools/llvm-as** — Assembles .ll to .bc (uses Writer)
- **Link-time optimization** — Uses Writer to generate .bc files for lazy loading
- **Any tool that writes .bc files**

## Notes for Developers

- **Record numbers**: Bitcode records are numbered in `llvm/include/llvm/Bitcode/LLVMBitCodes.h`. Maintain consistency with Reader.
- **Value IDs**: The Writer assigns IDs starting from 0. Understand the order (types, constants, functions, blocks, instructions) before changing enumeration.
- **Abbreviations**: Custom abbreviations compress bitcode significantly. Profile to find the best abbreviations.
- **Forward references**: Avoid emitting unnecessary forward reference. Emit in dependency order when possible.
- **Metadata handling**: Metadata is handled separately. Consult Reader's MetadataLoader for expectations.
- **Testing**: After writing bitcode, deserialize it with the Reader to verify round-trip correctness.
- **Performance**: Bitcode writing is on the critical path for LTO builds. Optimize hot code paths.

<!-- MANUAL: -->
