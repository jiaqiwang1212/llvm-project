<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/Support/rpmalloc

## Purpose

Vendored rpmalloc memory pool allocator. A fast, lock-free per-thread memory allocator designed for multi-threaded workloads. Used by LLVM's ThreadPool and other high-concurrency components to reduce allocation overhead and improve cache locality.

## Key Files

| File | Description |
|------|-------------|
| `rpmalloc.c` | Core rpmalloc implementation (per-thread heaps, lock-free allocation) |
| `rpmalloc.h` | Public rpmalloc API (malloc/free wrappers, thread management) |
| `rpnew.h` | C++ new/delete operator overloads |
| `malloc.c` | Standard malloc/free compatibility layer |
| `README.md` | rpmalloc usage guide and design notes |
| `CACHE.md` | Cache behavior and tuning documentation |
| `CMakeLists.txt` | rpmalloc build configuration |

## For AI Agents

### Working In This Directory

This is a **vendored third-party library**. Do not modify core rpmalloc unless fixing critical bugs or improving LLVM integration.

1. **Do not change allocation algorithm**: rpmalloc's lock-free design is carefully tuned. Changes can introduce race conditions or performance regressions.
2. **Thread-local storage**: rpmalloc uses thread-local heaps. Each thread allocates from its own pool without locking. Respect this design.
3. **Per-thread initialization**: Call `rpmalloc_initialize_thread()` when a thread starts using rpmalloc, and `rpmalloc_finalize_thread()` on cleanup.
4. **Memory pools**: rpmalloc organizes memory into size classes and spans. Understand these for troubleshooting.
5. **Compatibility layer**: `malloc.c` and `rpnew.h` wrap rpmalloc for standard C/C++ interfaces. These are safe to modify for LLVM integration.
6. **Upstream alignment**: Check rpmalloc's official repo (https://github.com/mjansson/rpmalloc) before making changes.

### Common Patterns

- **Per-thread initialization**: Call on thread creation, finalize on thread destruction.
- **Global initialization**: `rpmalloc_initialize()` once at program startup, `rpmalloc_finalize()` at shutdown.
- **Allocation**: Use standard `malloc()`, `free()`, or C++ `new`/`delete` — they dispatch to rpmalloc.
- **Memory stats**: Query with `rpmalloc_thread_statistics()` for debugging allocation patterns.
- **Cache locality**: rpmalloc optimizes for cache lines. Allocations from the same thread stay nearby.

## Dependencies

### Internal
- C standard library (string.h, stdatomic.h for lock-free operations, pthreads for threads)
- Platform-specific memory APIs (mmap/VirtualAlloc)

### External
- None (self-contained allocator)
- POSIX threads (pthreads) or Windows threading
- Standard C compiler with atomic operations support

<!-- MANUAL: -->
