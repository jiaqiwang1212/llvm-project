<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MSF

## Purpose
Multi-Stream File (MSF) container format used by Microsoft PDB files. Provides block-based file storage with multiple named streams. Low-level file format infrastructure for PDB reading and writing.

## Key Files
| File | Description |
|------|-------------|
| `MSFBuilder.cpp` | MSF file builder and writer |
| `MappedBlockStream.cpp` | Block stream mapping and random access |
| `MSFCommon.cpp` | Common MSF utilities and constants |
| `MSFError.cpp` | MSF error types and handling |

## For AI Agents

### Working In This Directory
- MSF is a container format; changes must respect block structure and stream layout.
- MappedBlockStream provides random access; performance is critical.
- MSFBuilder is the main API for creating MSF files; understand stream allocation before modifying.
- MSF is used exclusively by PDB; coordinate with `../PDB/` for integration.

### Dependencies

#### Internal
- Depends on `llvm/lib/Support/` (error handling, binary I/O)
- Used by `../PDB/` (PDB file format)
- Used by Windows debugging tools

<!-- MANUAL: -->
