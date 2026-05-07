<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# HTTP

## Purpose
Simple HTTP client and server functionality for network operations. Used by build systems, package managers, and remote caching backends.

## Key Files
| File | Description |
|------|-------------|
| `HTTPClient.h` | HTTP client for requests |
| `HTTPServer.h` | Minimal HTTP server |
| `StreamedHTTPResponseHandler.h` | Response streaming utilities |

## For AI Agents

### Working In This Directory
HTTP provides lightweight network operations:
- **HTTP client**: GET/POST requests with response handling
- **TLS support**: HTTPS via curl or system defaults
- **Response streaming**: Process large responses incrementally
- **Proxy support**: Configurable HTTP/HTTPS proxies

### Common Patterns
- HTTPClient for remote file fetching
- Request/response with headers and body
- Streaming responses to avoid memory buffering
- Error handling for network failures

## Dependencies

### Internal
- `llvm/Support` (Error, StringRef, raw_ostream)

### External
- libcurl (optional, for TLS)

<!-- MANUAL: -->
