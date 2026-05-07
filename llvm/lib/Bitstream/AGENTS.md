<!-- Generated: 2026-05-07 -->

# Bitstream — Low-Level Bit-Packed Container Format

## Purpose

Low-level infrastructure for reading and writing bit-packed container formats. Bitstream is a generic, language-agnostic format for storing nested blocks of records with custom abbreviations and alignment. LLVM Bitcode is built on top of Bitstream, but Bitstream itself is reusable for other formats.

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `Reader/` | Low-level bitstream deserialization — see `Reader/AGENTS.md` |

## For AI Agents

### Working In This Directory

1. **Generic abstraction**: Bitstream is format-agnostic. It handles only container structure (blocks, records, abbreviations), not semantics.

2. **Layered design**: Bitcode (LLVM IR serialization) is built on top of Bitstream. Changes to Bitstream affect all consumers.

3. **Block hierarchy**: Bitstreams consist of nested blocks. Each block has a numeric ID and contains records. Blocks can be recursive.

4. **Record abbreviations**: Records can be compressed using custom abbreviations defined per-block. Abbreviations are compact bit sequences.

5. **Bit-level efficiency**: Everything is packed at the bit level. Small changes can affect bitstream size significantly.

## Dependencies

### Internal

- **llvm/lib/Support/** — Bit manipulation and data utilities
- **llvm/include/llvm/Bitstream/** — Public bitstream headers

### External

None.

### Dependents

- **llvm/lib/Bitcode/** — Bitcode is built on Bitstream
- Any consumer of LLVM bitcode

## Notes for Developers

- **See subdirectories**: The Reader implementation is in `Reader/`. There is no Writer in this subdirectory; writers are in `llvm/include/llvm/Bitstream/BitstreamWriter.h`.
- **Header-only writer**: The bitstream Writer is typically header-only (in include/), while the Reader is in lib/.
- **Format specification**: The bitstream format is documented in the LLVM Language Reference.

<!-- MANUAL: -->
