<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bitstream

## Purpose

Low-level bitstream container for streaming binary data with variable-length encoding. Provides byte-aligned block structure, abbreviated records, and efficient bit/byte packing. Foundation for LLVM bitcode and other serialization formats.

## Key Files

| File | Description |
|------|-------------|
| `BitstreamReader.h` | Bitstream deserializer; block navigation, record reading, abbreviation expansion (19 KB) |
| `BitstreamWriter.h` | Bitstream serializer; record writing, abbreviation definition, block emission (25 KB) |
| `BitCodes.h` | Standard block/record codes (file ID, abbreviation definitions) (4 KB) |
| `BitCodeEnums.h` | Bitstream encoding enums (abbreviation op codes, type sizes) (3 KB) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding bitstream encoding (VBR, fixed-width, aligned fields)
- Analyzing block nesting and record structure
- Reviewing abbreviation compression strategies

**Implementation tasks:**
- Optimizing encoding for common record patterns
- Adding support for new record types
- Improving streaming performance (e.g., prefetching for large blocks)

### Common Patterns

1. **VBR encoding**: Variable-bit-rate integers (1-9 bits per byte); saves space for small numbers
2. **Block structure**: Named blocks (BLOCKINFO, FUNCTION_BLOCK, etc.) with parentIDs; enable selective parsing
3. **Abbreviations**: Reusable record templates reduce repeated field encoding; aggressive compression
4. **Record codes**: First field identifies record type; remaining fields encode data
5. **Byte alignment**: Block/record boundaries align to byte boundaries for efficient seeking

### Streaming Model

- **Top-down parsing**: Reader enters blocks in sequence; can skip blocks without reading contents
- **Lazy abbreviation lookup**: Abbreviations defined in BLOCKINFO block; reused in all blocks
- **Fixed block size**: Block header contains bitcount; enables exact seeking to next block

### Important Notes

- **Embedded in Bitcode**: BitstreamReader/Writer used by llvm/Bitcode/ for LLVM IR serialization
- **Generic design**: Not specific to LLVM; suitable for any hierarchical binary format
- **Stable format**: Changes here affect bitcode versioning; coordinate carefully

## Dependencies

### Internal
- `llvm/Support/` — Error, Endianness, DataExtractor, raw_ostream

### External
- None

<!-- MANUAL: -->
