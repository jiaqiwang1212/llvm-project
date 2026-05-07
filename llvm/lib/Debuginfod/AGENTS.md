<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Debuginfod

## Purpose

HTTP client implementation for debuginfod protocol. Enables tools to fetch missing debug information (.debug_info sections, source files, separate debug symbols) from remote debuginfod servers.

## Key Files

| File | Description |
|------|-------------|
| `BuildIDFetcher.cpp` | Fetches debug artifacts by build ID |
| `Debuginfod.cpp` | Main debuginfod HTTP client |

## For AI Agents

### Working In This Directory

1. Understand debuginfod protocol (https://sourceware.org/debuginfod/)
2. Know build ID format and lookup mechanism
3. Test fetching debug symbols from public debuginfod servers
4. Handle network errors and fallback strategies
5. Verify certificate validation and security
6. Cache fetched artifacts locally
7. Support both HTTP and HTTPS

### Key Patterns

- Build ID uniquely identifies debug information
- HTTP GET request to debuginfod server endpoint
- Response is raw ELF file with debug sections
- Caching prevents repeated network fetches
- Fallback to alternative servers on failure

## Dependencies

### Internal
- Depends on: LLVM Support, system HTTP/networking
- Used by: Debugging tools, symbolizers, LLDB

<!-- MANUAL: -->
