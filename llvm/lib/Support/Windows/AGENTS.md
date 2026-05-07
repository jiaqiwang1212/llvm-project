<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/Support/Windows

## Purpose

Windows-specific platform abstraction implementations. Provides Win32 implementations of cross-platform APIs defined in Support.cpp (file system, process management, signals, threading, dynamic libraries, memory management). Included only on Windows; Unix-like systems use the Unix/ sibling directory.

## Key Files

| File | Description |
|------|-------------|
| `Path.inc` | Windows path manipulation (backslash handling, drive letters, UNC paths, GetFullPathName) |
| `Process.inc` | Windows process utilities (get executable path, env vars, user info, GetCurrentProcessId) |
| `Program.inc` | Windows program execution (CreateProcess, WaitForSingleObject, exit codes) |
| `Signals.inc` | Windows signal emulation (SetConsoleCtrlHandler for Ctrl+C, exception handlers) |
| `Threading.inc` | Windows thread management (CreateThread, WaitForSingleObject, CriticalSection, Event) |
| `Memory.inc` | Windows memory management (VirtualAlloc, VirtualProtect, page allocation) |
| `DynamicLibrary.inc` | Windows dynamic library loading (LoadLibrary, GetProcAddress, FreeLibrary) |
| `Watchdog.inc` | Windows timeout/watchdog (CreateWaitableTimer, WaitForSingleObject) |
| `Jobserver.inc` | Windows jobserver integration (named pipes, overlapped I/O) |
| `COM.inc` | Windows COM (Component Object Model) support (CoInitialize, QueryInterface stubs) |
| `explicit_symbols.inc` | Windows explicit symbol export list |

## For AI Agents

### Working In This Directory

These files provide Windows-specific implementations of abstract platform APIs. When working here:

1. **Windows API patterns**: Use modern Win32 APIs. Understand HANDLE, GetLastError(), and LPSTR/LPWSTR string conventions.
2. **Unicode handling**: Windows uses UTF-16 internally. Handle ANSI/Unicode conversions carefully (MultiByteToWideChar, WideCharToMultiByte).
3. **Error handling**: Use GetLastError() and convert to HRESULT or error codes. Include Windows error codes in diagnostics.
4. **RAII**: Close all HANDLEs (files, processes, events, mutexes) in destructors. Use CloseHandle consistently.
5. **Synchronization**: CriticalSection for mutexes, Event/WaitableTimer for condition variables, CreateProcess for spawning.
6. **Path handling**: Account for drive letters (C:), UNC paths (\\server\share), backslashes, and relative paths.

### Common Patterns

- **Handle management**: Store HANDLEs, check for INVALID_HANDLE_VALUE, close with CloseHandle.
- **Error codes**: GetLastError() captures per-thread error. Convert to HRESULT or errno equivalent.
- **Unicode conversions**: Use MultiByteToWideChar for UTF-8 to UTF-16, WideCharToMultiByte for reverse.
- **Process creation**: CreateProcess with CREATE_NO_WINDOW or CREATE_NEW_PROCESS_GROUP for backgrounding.
- **Thread synchronization**: InitializeCriticalSection, EnterCriticalSection, LeaveCriticalSection.
- **Memory mapping**: VirtualAlloc for allocation, VirtualProtect for protection, VirtualFree for cleanup.

## Dependencies

### Internal
- LLVM headers: `llvm/Support/Error.h`, `llvm/Support/StringRef.h`, `llvm/Support/Mutex.h`
- This directory is included from `Support.cpp` with proper #ifdef guards

### External
- Windows API (winbase.h, winnt.h, winerror.h, process.h, io.h)
- Standard C library (string.h, stdlib.h, errno.h)
- Windows Subsystem (kernel32.dll, advapi32.dll, etc.)

<!-- MANUAL: -->
