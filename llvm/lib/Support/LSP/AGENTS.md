<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/Support/LSP

## Purpose

Language Server Protocol (LSP) transport and communication utilities. Provides JSON-RPC protocol handling, message encoding/decoding, and transport abstractions for LSP clients and servers. Used by LLVM tools that implement LSP (clangd, etc.).

## Key Files

| File | Description |
|------|-------------|
| `Protocol.cpp` | LSP protocol message types, JSON serialization/deserialization |
| `Transport.cpp` | JSON-RPC transport layer (stdio, socket, bidirectional pipes) |
| `Logging.cpp` | LSP logging and diagnostics output |
| `CMakeLists.txt` | LSP library build configuration |

## For AI Agents

### Working In This Directory

This library bridges LLVM's internal data structures and the Language Server Protocol. When working here:

1. **Protocol compliance**: LSP is defined by the official specification. Ensure message formats match the spec exactly.
2. **JSON serialization**: All protocol messages serialize to/from JSON. Use consistent field naming (camelCase per LSP spec).
3. **Bidirectional communication**: Transport must handle both sending and receiving. Assume messages can arrive at any time.
4. **Error handling**: LSP errors use specific error codes (parse error, invalid request, server error ranges). Respect these categories.
5. **Message ordering**: LSP doesn't guarantee message order for notifications, but requests must have ordered responses.
6. **Resource management**: Transports may use pipes, sockets, or stdio. Clean shutdown is critical.

### Common Patterns

- **Message creation**: Build protocol message objects, serialize to JSON-RPC with message ID.
- **Request/response**: Match response ID to request ID. Handle timeouts for long-running operations.
- **Notifications**: Fire-and-forget messages (no response ID). Examples: didOpen, didChange, didClose.
- **Error responses**: Send error responses with standard LSP error codes and messages.
- **JSON-RPC format**: `{"jsonrpc": "2.0", "id": N, "method": "...", "params": {...}}` for requests.

## Dependencies

### Internal
- `llvm/Support/Error.h` — LLVM error handling
- `llvm/Support/JSON.h` — JSON parsing and generation
- `llvm/Support/raw_ostream.h` — Stream I/O
- `llvm/Support/StringRef.h` — String references
- `llvm/Support/Threading.h` — Thread utilities (if async I/O)

### External
- None (self-contained transport implementation)
- Standard C++ library

<!-- MANUAL: -->
