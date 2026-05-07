<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# HTTP

## Purpose
Simple HTTP client and server implementation for LLVM tools. Provides both request handling and streaming response support without external dependencies (uses platform APIs or libcurl).

## Key Files
| File | Description |
|------|-------------|
| `HTTPClient.cpp` | HTTP client for making requests |
| `HTTPServer.cpp` | HTTP server implementation |
| `StreamedHTTPResponseHandler.cpp` | Streaming response handling |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- HTTP client/server abstraction for tool communication
- Used by Debuginfod and other debug infrastructure
- Streaming support for large responses
- Cross-platform implementation (Windows/Unix)

## Dependencies

### Internal
- `llvm/lib/Support/` — error handling, string utilities

<!-- MANUAL: -->
