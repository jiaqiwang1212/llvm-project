<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCopy/DXContainer

## Purpose

Implements object file transformation for DirectX container format files. Handles reading, modifying, and writing DXContainer files which bundle compiled shaders and shader metadata for DirectX.

## Key Files

| File | Description |
|------|-------------|
| `DXContainerObjcopy.cpp` | Main DXContainer transformation logic |
| `DXContainerObject.cpp` | DXContainer object representation |
| `DXContainerObject.h` | DXContainer object interface definition |
| `DXContainerReader.cpp` | DXContainer file parsing |
| `DXContainerReader.h` | DXContainer reader interface |
| `DXContainerWriter.cpp` | DXContainer file serialization |
| `DXContainerWriter.h` | DXContainer writer interface |

## For AI Agents

### Working In This Directory

1. Understand DirectX container file structure and part types
2. Know shader metadata encoding and validation requirements
3. Test with DirectX shader binaries and shader libraries
4. Verify part headers, section offsets, and checksums
5. Handle part reordering and modification
6. Coordinate with HLSL utilities (lib/Frontend/HLSL)

### Key Patterns

- DXContainer consists of typed "parts" (shader code, reflection metadata, etc.)
- Each part has header and potentially nested structure
- Reader/writer maintain part offsets and indices
- Metadata must be updated when code sections change

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM BinaryFormat, ObjCopy common infrastructure
- Used by: llvm-objcopy when processing DirectX shader containers

<!-- MANUAL: -->
