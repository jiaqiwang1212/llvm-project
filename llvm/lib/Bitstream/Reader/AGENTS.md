<!-- Generated: 2026-05-07 -->

# Bitstream/Reader — Low-Level Bitstream I/O

## Purpose

Low-level bitstream container reading. Provides the foundation for bitcode format: reads bit-packed data, handles record abbreviations, manages alignment, and provides utilities for block-based record streams. The Reader class handles navigation through bitstream blocks and records without interpreting their meaning.

## Key Files

| File | Description |
|------|-------------|
| `BitstreamReader.cpp` | Low-level bitstream reading (block and record navigation) |

## Subdirectories

None. This is a leaf directory under `llvm/lib/Bitstream/`.

## For AI Agents

### Working In This Directory

1. **Abstraction level**: Bitstream Reader is generic—it doesn't know about LLVM IR. It only handles the container format (blocks, records, abbreviations). Higher-level code (Bitcode Reader) interprets the contents.

2. **Blocks and records**: Bitstreams are organized as nested blocks, containing records. Each record has an ID and operands. Understand this structure.

3. **Abbreviations**: Custom abbreviations compress records. The Reader must handle abbreviations consistently with the Writer.

4. **Bit alignment**: Blocks and records have specific alignment requirements. The Reader enforces this.

5. **Error handling**: Corrupted bitstreams must be detected early. Validate boundaries and checksums.

6. **Performance**: Bitstream reading is fast but fundamental. Small optimizations here have large downstream impact.

### Common Patterns

**Reading a block:**
```cpp
// BitstreamReader manages block entry/exit
BitstreamCursor Cursor(Buffer);
unsigned BlockID = Cursor.ReadSubBlockID();
unsigned BlockLen = Cursor.ReadVBR(32);
// Process block contents
Cursor.ExitBlock();
```

**Reading a record:**
```cpp
SmallVector<uint64_t, 64> Record;
unsigned Code = Cursor.ReadRecord(AbbrevID, Record);
// Process record based on Code
```

**Handling abbreviations:**
```cpp
// Writer defines abbreviations with BitCodeAbbrev
// Reader reads abbreviation definitions from stream
// Reader applies abbreviations when reading records
```

## Dependencies

### Internal

- **llvm/lib/Support/** — Bit manipulation and data utilities
- **llvm/include/llvm/Bitstream/Bitstream.h** — Public bitstream header

### External

None.

### Dependents

- **llvm/lib/Bitcode/Reader/** — Uses BitstreamReader to read bitcode records
- **llvm/lib/Bitcode/Writer/** — Produces bitstream that BitstreamReader consumes
- Any tool that reads bit-packed container formats

## Notes for Developers

- **No interpretation**: This module does not interpret record semantics. That's the Bitcode Reader's job.
- **Generic design**: The bitstream format is language-agnostic. It could theoretically be used for other formats.
- **VBR encoding**: Variable-byte-rate (VBR) encoding is used for integers. Understand how VBR works before modifying.
- **Block numbering**: Blocks have numeric IDs. Reserved block IDs (0-7) have special meanings (MODULE_BLOCK, etc.). Consult `LLVMBitCodes.h` for the mapping.
- **Record abbreviations**: Abbreviations are defined per-block. When exiting a block, all its abbreviations are lost.
- **Testing**: Test bitstream reading with both manually-created and writer-generated streams.

<!-- MANUAL: -->
