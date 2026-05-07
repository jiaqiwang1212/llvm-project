<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# WindowsManifest

## Purpose
Windows application manifest XML generation and embedding. Enables embedding manifest resources in executables to specify runtime dependencies, privileges, and compatibility requirements.

## Key Files
| File | Description |
|------|-------------|
| `WindowsManifestMerger.cpp` | Manifest merging and XML generation |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Manifest XML format specification
- Multi-version manifest merging
- Resource embedding for executables
- Dependency declaration and versioning

## Dependencies

### Internal
- `llvm/lib/Support/` — utilities, XML parsing

<!-- MANUAL: -->
