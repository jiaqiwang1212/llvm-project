<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/Support/BLAKE3

## Purpose

Vendored BLAKE3 cryptographic hash implementation. BLAKE3 is a modern, fast cryptographic hash function used by LLVM for content-based hashing (e.g., reproducible builds, deduplication). Includes optimized SIMD implementations for AVX2, AVX512, SSE2, SSE4.1, and ARM NEON, with portable C fallback.

## Key Files

| File | Description |
|------|-------------|
| `blake3.c` | Main BLAKE3 algorithm (portable core) |
| `blake3.h` | Public BLAKE3 API (not committed, generated) |
| `blake3_portable.c` | Portable C implementation for all platforms |
| `blake3_dispatch.c` | SIMD dispatcher (detects CPU features, selects fastest path) |
| `blake3_avx2.c` | AVX2 SIMD implementation (256-bit vectors) |
| `blake3_avx512.c` | AVX512 SIMD implementation (512-bit vectors) |
| `blake3_sse2.c` | SSE2 SIMD implementation (128-bit vectors) |
| `blake3_sse41.c` | SSE4.1 SIMD implementation |
| `blake3_neon.c` | ARM NEON SIMD implementation |
| `blake3_avx2_x86-64_unix.S` | AVX2 assembly for Unix x86-64 |
| `blake3_avx2_x86-64_windows_gnu.S` | AVX2 assembly for Windows GNU x86-64 |
| `blake3_avx2_x86-64_windows_msvc.asm` | AVX2 assembly for Windows MSVC x86-64 |
| `blake3_avx512_x86-64_unix.S` | AVX512 assembly for Unix x86-64 |
| `blake3_avx512_x86-64_windows_gnu.S` | AVX512 assembly for Windows GNU x86-64 |
| `blake3_avx512_x86-64_windows_msvc.asm` | AVX512 assembly for Windows MSVC x86-64 |
| `blake3_sse2_x86-64_unix.S` | SSE2 assembly for Unix x86-64 |
| `blake3_sse2_x86-64_windows_gnu.S` | SSE2 assembly for Windows GNU x86-64 |
| `blake3_sse2_x86-64_windows_msvc.asm` | SSE2 assembly for Windows MSVC x86-64 |
| `blake3_sse41_x86-64_unix.S` | SSE4.1 assembly for Unix x86-64 |
| `blake3_sse41_x86-64_windows_gnu.S` | SSE4.1 assembly for Windows GNU x86-64 |
| `blake3_sse41_x86-64_windows_msvc.asm` | SSE4.1 assembly for Windows MSVC x86-64 |
| `blake3_impl.h` | Implementation selection macros |
| `llvm_blake3_prefix.h` | LLVM namespace wrapping and integration |
| `CMakeLists.txt` | BLAKE3 build configuration |
| `LICENSE` | BLAKE3 license (CC0 public domain) |
| `README.md` | BLAKE3 vendoring notes |

## For AI Agents

### Working In This Directory

This is a **vendored third-party library**. Do not modify BLAKE3 algorithm or core implementations unless fixing critical bugs.

1. **Do not change algorithm**: BLAKE3 is frozen by design. Changes break compatibility and reproducibility.
2. **Integration point**: `llvm_blake3_prefix.h` wraps symbols into LLVM namespace. This is safe to modify for LLVM integration.
3. **SIMD selection**: `blake3_dispatch.c` auto-detects CPU features and selects the fastest implementation. Add new SIMD paths here only if adding CPU support.
4. **Assembly**: Platform-specific `.S` and `.asm` files are optimized implementations. Only modify for new platform support or critical fixes.
5. **Portable fallback**: `blake3_portable.c` must work on all platforms. Changes here affect everyone.
6. **Upstream updates**: Check official BLAKE3 repo (https://github.com/BLAKE3-team/BLAKE3) before making changes. Prefer upstreaming fixes.

### Common Patterns

- **Hash computation**: Instantiate `blake3_hasher`, feed data with `blake3_hasher_update()`, finalize with `blake3_hasher_finalize()`.
- **SIMD dispatch**: `blake3_dispatch.c` selects implementation at runtime. No manual selection needed.
- **Incremental hashing**: `blake3_hasher` supports streaming (update multiple times before finalize).
- **Variable output**: BLAKE3 supports arbitrary output lengths. Use finalization mode to control length.

## Dependencies

### Internal
- C standard library (string.h, stdint.h, etc.)
- `llvm_blake3_prefix.h` for LLVM integration

### External
- SIMD instruction sets (AVX2, AVX512, SSE2, SSE4.1, NEON) — auto-detected and optional
- No external libraries required

<!-- MANUAL: -->
