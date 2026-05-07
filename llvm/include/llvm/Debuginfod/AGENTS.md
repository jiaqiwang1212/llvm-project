<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Debuginfod

## Purpose

Client for debuginfod protocol (HTTP-based debug info distribution). Fetches debug symbols, source files, and executable binaries from federated debuginfod servers. Enables symbol resolution for crashes and performance analysis without local symbol installation.

## Key Files

| File | Description |
|------|-------------|
| `Debuginfod.h` | Client interface; build ID lookup, download, caching (6 KB) |
| `BuildIDFetcher.h` | Build ID extractor for ELF binaries (1 KB) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding debuginfod protocol (build ID queries, HTTP endpoints)
- Reviewing caching strategies for symbol downloads
- Analyzing cross-server fallback logic

**Implementation tasks:**
- Adding new debuginfod server backends
- Implementing cache eviction policies
- Enhancing parallel fetch for multi-server environments

### Common Patterns

1. **Build ID query**: Client extracts build ID from binary, queries servers for matching debug artifact
2. **HTTP fallback**: Primary server fails -> try secondary servers (federated model)
3. **Local caching**: Downloaded symbols cached locally to avoid repeated HTTP requests
4. **Source file retrieval**: In addition to debug sections, fetch source code for symbolization
5. **Offline fallback**: Gracefully falls back to local symbol files if servers unavailable

### Protocol Notes

- **RESTful endpoints**: `/buildid/<BUILD_ID>/debuginfo`, `/buildid/<BUILD_ID>/executable`, `/buildid/<BUILD_ID>/source/<PATH>`
- **Build ID format**: Hex-encoded SHA1 (40 chars) extracted from ELF .note.gnu.build-id section
- **Server configuration**: DEBUGINFOD_URLS environment variable specifies server list

## Dependencies

### Internal
- `llvm/Object/` — Binary/ObjectFile for build ID extraction
- `llvm/Support/` — Error, HTTP client utilities

### External
- libcurl (optional; HTTP transport)

<!-- MANUAL: -->
