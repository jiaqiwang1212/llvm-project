<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/Support/Unix

## Purpose

POSIX-specific platform abstraction implementations. Provides Unix/Linux implementations of cross-platform APIs defined in Support.cpp (file system, process management, signals, threading, dynamic libraries, memory management). Included only on Unix-like systems; Windows uses the Windows/ sibling directory.

## Key Files

| File | Description |
|------|-------------|
| `Path.inc` | POSIX path manipulation (join, dirname, absolute paths, realpath) |
| `Process.inc` | POSIX process utilities (get executable path, env vars, user info, getpid) |
| `Program.inc` | POSIX program execution (fork, execve, wait, signal handling) |
| `Signals.inc` | POSIX signal handlers (SIGINT, SIGSEGV, cleanup, backtrace) |
| `Threading.inc` | POSIX thread management (pthread create, join, mutex, condition vars) |
| `Memory.inc` | POSIX memory management (mmap, mprotect, page allocation) |
| `DynamicLibrary.inc` | POSIX dynamic library loading (dlopen, dlsym, dlclose) |
| `Watchdog.inc` | POSIX timeout/watchdog (alarm, timer, signal-based) |
| `Jobserver.inc` | GNU make jobserver integration (named pipes, fd communication) |
| `COM.inc` | POSIX COM (Component Object Model) stub (no-op on Unix) |
| `Unix.h` | Unix-specific helper functions and macros |
| `README.txt` | Unix platform-specific notes |

## For AI Agents

### Working In This Directory

These files provide POSIX implementations of abstract platform APIs. When working here:

1. **Portability first**: Code must work on Linux, macOS, *BSD, and other POSIX systems. Use standard POSIX APIs, not GNU extensions unless necessary.
2. **Error handling**: Use `errno` consistently. Convert to `std::error_code` or `llvm::Error` for LLVM APIs.
3. **Resource cleanup**: RAII is critical. Ensure file descriptors, memory, and threads are cleaned up on exception or early return.
4. **Signal safety**: Functions called from signal handlers must be async-signal-safe. See man signal-safety(7).
5. **Thread safety**: All file-level state must be thread-safe. Use `llvm::sys::Mutex` for synchronization.
6. **Blocking vs non-blocking**: Program.inc may use blocking system calls. Understand implications for multi-threaded use.

### Common Patterns

- **File descriptor management**: Open with proper flags (O_CLOEXEC for close-on-exec), close in destructors.
- **Error propagation**: Convert errno to error codes before returning to caller.
- **Signal handlers**: Keep minimal, use signal-safe functions only (write, not printf).
- **Thread creation**: Use pthread_create, join with pthread_join, set stack size with pthread_attr.
- **Memory mapping**: Use mmap for large files, mprotect for protection, munmap for cleanup.
- **Process spawning**: Use fork/execve pattern, handle SIGCHLD for reaping zombies.

## Dependencies

### Internal
- LLVM headers: `llvm/Support/Error.h`, `llvm/Support/StringRef.h`, `llvm/Support/Mutex.h`
- This directory is included from `Support.cpp` with proper #ifdef guards

### External
- POSIX APIs (unistd.h, fcntl.h, sys/mman.h, pthread.h, signal.h, dlfcn.h)
- Standard C library (string.h, stdlib.h, errno.h)
- Linux/Unix kernel headers (for process, file system, memory management)

<!-- MANUAL: -->
