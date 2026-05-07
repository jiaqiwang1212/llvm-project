<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# WindowsManifest

## Purpose
Windows application manifest embedding and merging. Creates and combines manifest XML for Windows compatibility requirements and side-by-side assembly specifications.

## Key Files
| File | Description |
|------|-------------|
| `WindowsManifestMerger.h` | Merge multiple manifest files |

## For AI Agents

### Working In This Directory
WindowsManifest handles Windows manifest operations:
- **Manifest XML**: Create or parse Windows application manifests
- **Merging**: Combine manifests from multiple sources
- **Embedding**: Embed manifests in executables and DLLs
- **Version info**: Specify runtime dependencies and UAC requirements

### Common Patterns
- Parse XML manifest documents
- Merge manifests with conflict resolution
- Embed merged manifest into PE executable
- Validate manifest structure and completeness

## Dependencies

### Internal
- `llvm/Support` (StringRef, Error, raw_ostream)
- XML parsing library (TinyXML or similar)

### External
- Windows API (manifest resources)

<!-- MANUAL: -->
