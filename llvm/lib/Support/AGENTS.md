<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/Support

## Purpose

Core utility library used throughout LLVM. Provides fundamental abstractions for error handling, memory management, data structures, I/O streams, string processing, numeric types (APInt, APFloat), file system access, process management, threading, and platform abstraction (Unix/Windows).

## Key Files

| File | Category | Description |
|------|----------|-------------|
| `Error.cpp` | Error Handling | LLVM error handling system with llvm::Error and llvm::Expected types |
| `ErrorHandling.cpp` | Error Handling | Crash recovery, assertions, and error callbacks |
| `raw_ostream.cpp` | I/O | Raw output stream abstraction (formatting, buffering) |
| `raw_os_ostream.cpp` | I/O | OS-backed output streams |
| `raw_socket_stream.cpp` | I/O | Socket-based output streams |
| `MemoryBuffer.cpp` | I/O | In-memory file buffers with lazy-loading |
| `FileOutputBuffer.cpp` | I/O | Memory-mapped file output |
| `SourceMgr.cpp` | I/O | Source location tracking and diagnostics |
| `APInt.cpp` | Numeric | Arbitrary-precision integer arithmetic |
| `APFloat.cpp` | Numeric | Arbitrary-precision floating-point |
| `APFixedPoint.cpp` | Numeric | Fixed-point number representation |
| `APSInt.cpp` | Numeric | Signed APInt wrapper |
| `DynamicAPInt.cpp` | Numeric | Dynamic-length arbitrary-precision integers |
| `SlowDynamicAPInt.cpp` | Numeric | Fallback slow APInt implementation |
| `Allocator.cpp` | Memory | Slab allocator for bump allocation |
| `MemAlloc.cpp` | Memory | Memory allocation wrapper |
| `BuryPointer.cpp` | Memory | Pointer burial (leak cleanup) |
| `StringRef.cpp` | String | Non-owning string reference type |
| `StringMap.cpp` | String | Hash map for string keys |
| `StringExtras.cpp` | String | String utilities (split, trim, conversions) |
| `StringSaver.cpp` | String | String deduplication and pooling |
| `Twine.cpp` | String | String concatenation abstraction |
| `FormatVariadic.cpp` | String | Printf-style formatting |
| `NativeFormatting.cpp` | String | Low-level format conversion |
| `Regex.cpp` | String | POSIX regex support |
| `ConvertUTF.cpp` | Encoding | UTF-8/16/32 conversion |
| `ConvertUTFWrapper.cpp` | Encoding | Wrapper for UTF conversion |
| `ConvertEBCDIC.cpp` | Encoding | EBCDIC conversion |
| `Unicode.cpp` | Encoding | Unicode utilities |
| `UnicodeCaseFold.cpp` | Encoding | Unicode case-folding tables |
| `UnicodeNameToCodepoint.cpp` | Encoding | Unicode name lookup |
| `TextEncoding.cpp` | Encoding | Text encoding detection |
| `Locale.cpp` | Encoding | Locale support |
| `CommandLine.cpp` | CLI | Command-line argument parsing and configuration |
| `Process.cpp` | System | Process utilities (path, env vars, user info) |
| `Program.cpp` | System | Program execution (fork/exec) |
| `Signals.cpp` | System | Signal handling (ctrl-C, crashes) |
| `Threading.cpp` | System | Thread creation and management |
| `ThreadPool.cpp` | System | Thread pool with work queue |
| `DynamicLibrary.cpp` | System | Dynamic library loading |
| `PluginLoader.cpp` | System | Plugin loading framework |
| `Watchdog.cpp` | System | Timeout/watchdog timer |
| `Path.cpp` | FileSystem | Path manipulation (join, dirname, extension) |
| `FileSystem.cpp` | FileSystem | File operations (stat, mkdir, iterate dirs) |
| `VirtualFileSystem.cpp` | FileSystem | Pluggable file system abstraction |
| `FileCollector.cpp` | FileSystem | VFS collector for reproducible builds |
| `FileUtilities.cpp` | FileSystem | File comparison and utilities |
| `GlobPattern.cpp` | FileSystem | Glob pattern matching |
| `MemoryBuffer.cpp` | FileSystem | File reading into memory |
| `Memory.cpp` | System | Memory page allocation and protection |
| `LockFileManager.cpp` | FileSystem | File locking for incremental builds |
| `Parallel.cpp` | Concurrency | Parallel for loop utilities |
| `Jobserver.cpp` | Concurrency | GNU make jobserver integration |
| `ExponentialBackoff.cpp` | Utilities | Exponential backoff retry logic |
| `Hash.cpp` | Hash | Generic hashing utilities |
| `MD5.cpp` | Hash | MD5 implementation |
| `SHA1.cpp` | Hash | SHA-1 implementation |
| `SHA256.cpp` | Hash | SHA-256 implementation |
| `SipHash.cpp` | Hash | SipHash implementation |
| `DJB.cpp` | Hash | DJB hash algorithm |
| `xxhash.cpp` | Hash | xxHash implementation |
| `CRC.cpp` | Hash | CRC checksum |
| `FoldingSet.cpp` | DataStructure | Hash-based folding set for CSE |
| `SmallVector.cpp` | DataStructure | Small vector small-size optimization |
| `SmallPtrSet.cpp` | DataStructure | Small pointer set |
| `IntervalMap.cpp` | DataStructure | Interval-based map |
| `IntEqClasses.cpp` | DataStructure | Integer equivalence classes |
| `IntegerInclusiveInterval.cpp` | DataStructure | Integer interval representation |
| `TrieRawHashMap.cpp` | DataStructure | Trie-based hash map |
| `DeltaAlgorithm.cpp` | Algorithm | Delta debugging algorithm |
| `DAGDeltaAlgorithm.cpp` | Algorithm | DAG-based delta debugging |
| `DeltaTree.cpp` | Algorithm | Delta tree for text diffs |
| `SuffixTree.cpp` | Algorithm | Suffix tree construction |
| `SuffixTreeNode.cpp` | Algorithm | Suffix tree node utilities |
| `BalancedPartitioning.cpp` | Algorithm | Graph partitioning algorithm |
| `GraphWriter.cpp` | Utilities | GraphViz DOT graph generation |
| `ScaledNumber.cpp` | Numeric | Scaled number representation |
| `BlockFrequency.cpp` | Numeric | Block frequency analysis |
| `BranchProbability.cpp` | Numeric | Branch probability representation |
| `KnownBits.cpp` | Analysis | Known bits tracking |
| `KnownFPClass.cpp` | Analysis | Known floating-point class tracking |
| `MathExtras.cpp` | Utilities | Math helper functions |
| `Timer.cpp` | Profiling | Timing utilities |
| `TimeProfiler.cpp` | Profiling | Time profiling infrastructure |
| `Statistic.cpp` | Profiling | Statistics collection and reporting |
| `Debug.cpp` | Debug | Debug output control |
| `DebugCounter.cpp` | Debug | Debug counter for selective execution |
| `PrettyStackTrace.cpp` | Debug | Pretty stack trace printing |
| `CrashRecoveryContext.cpp` | Debug | Crash recovery and cleanup |
| `Valgrind.cpp` | Debug | Valgrind integration |
| `Signposts.cpp` | Debug | OS signpost logging |
| `Chrono.cpp` | Utilities | Time/duration utilities |
| `Errno.cpp` | System | Errno utilities |
| `Optional.cpp` | Utilities | Optional value type support |
| `Mustache.cpp` | Utilities | Mustache template engine |
| `YAMLParser.cpp` | Format | YAML parsing |
| `YAMLTraits.cpp` | Format | YAML serialization traits |
| `JSON.cpp` | Format | JSON parsing and generation |
| `Base64.cpp` | Encoding | Base64 encoding/decoding |
| `Compression.cpp` | Compression | Zlib and other compression wrappers |
| `TarWriter.cpp` | Archive | Tar archive writing |
| `DataExtractor.cpp` | Binary | Binary data extraction utilities |
| `BinaryStreamReader.cpp` | Binary | Binary stream reading |
| `BinaryStreamWriter.cpp` | Binary | Binary stream writing |
| `BinaryStreamRef.cpp` | Binary | Binary stream references |
| `BinaryStreamError.cpp` | Binary | Binary stream error handling |
| `LEB128.cpp` | Binary | LEB128 variable-length encoding |
| `RewriteBuffer.cpp` | Utilities | Text rewriting with offset tracking |
| `RewriteRope.cpp` | Utilities | Rope data structure for text |
| `LineIterator.cpp` | Utilities | Line-by-line iteration |
| `InitLLVM.cpp` | Utilities | LLVM initialization |
| `Version.cpp` | Utilities | Version information |
| `VersionTuple.cpp` | Utilities | Version tuple representation |
| `ManagedStatic.cpp` | Utilities | Global static object management |
| `ExtensibleRTTI.cpp` | RTTI | Extensible RTTI system |
| `TargetRegistry.cpp` | Registry | Target registration system |
| `DynamicLibrary.cpp` | System | Dynamic library loading |
| `Caching.cpp` | Caching | Caching utilities for build systems |
| `CachePruning.cpp` | Caching | Cache pruning strategies |
| `SystemUtils.cpp` | System | System utility functions |
| `FormattedStream.cpp` | I/O | Formatted output stream |
| `circular_raw_ostream.cpp` | I/O | Circular buffer output stream |
| `raw_ostream_proxy.cpp` | I/O | Output stream proxy wrapper |
| `FileCollector.cpp` | Utilities | VFS file collection |
| `FloatingPointMode.cpp` | System | Floating-point mode control |
| `Z3Solver.cpp` | SMT | Z3 SMT solver integration |
| `SpecialCaseList.cpp` | Text | Regex-based filtering lists |
| `DXILMetadata.cpp` | Format | DXIL metadata support |
| `DXILABI.cpp` | Format | DXIL ABI information |
| `ModRef.cpp` | Analysis | Mod/Ref analysis utilities |
| `InstructionCost.cpp` | Analysis | Instruction cost tracking |
| `OptimizedStructLayout.cpp` | Utilities | Struct layout optimization |
| `CachePruning.cpp` | Caching | Cache pruning |
| `CodeGenCoverage.cpp` | Coverage | Code generation coverage tracking |
| `ProgramStack.cpp` | Debug | Program stack utilities |
| `OptionStrCmp.cpp` | CLI | Option string comparison |
| `DivisionByConstantInfo.cpp` | Numeric | Constant division optimization |
| `DynamicAPInt.cpp` | Numeric | Dynamic arbitrary-precision integers |
| `ELFAttributes.cpp` | Format | ELF attribute handling |
| `WithColor.cpp` | I/O | Colored terminal output |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `BLAKE3/` | BLAKE3 cryptographic hash (vendored C) — see `BLAKE3/AGENTS.md` |
| `LSP/` | Language Server Protocol utilities — see `LSP/AGENTS.md` |
| `rpmalloc/` | rpmalloc pool allocator (vendored C) — see `rpmalloc/AGENTS.md` |
| `Unix/` | POSIX platform-specific implementations — see `Unix/AGENTS.md` |
| `Windows/` | Windows platform-specific implementations — see `Windows/AGENTS.md` |

## For AI Agents

### Working In This Directory

This is LLVM's foundational utility library. When working here:

1. **Understand the layering**: Low-level utilities (memory, strings, errors) are used by everything above. Changes here affect the entire compiler.
2. **Platform abstraction**: Features split between `Support.cpp` (generic) and `Unix/*.inc` or `Windows/*.inc` (platform-specific). Always implement both.
3. **Error handling**: Use `llvm::Error` and `llvm::Expected<T>` consistently. Avoid exceptions (LLVM code doesn't use them).
4. **Memory safety**: Prefer RAII, use smart pointers (unique_ptr, intrusive_ptr). Avoid raw new/delete.
5. **String handling**: Use `StringRef` (non-owning), `Twine` (concatenation), `std::string` (owning). Never raw char*.
6. **Data structures**: SmallVector for most containers (size optimization), SmallPtrSet for pointer sets, IntervalMap for overlapping ranges.

### Common Patterns

- **Numeric types**: `APInt` / `APFloat` for compile-time constants, `DynamicAPInt` for runtime scalability.
- **String concatenation**: Use `Twine` for lazy concatenation, avoid repeated `std::string +=`.
- **File I/O**: Use `MemoryBuffer` for reading, `FileOutputBuffer` for atomic writes.
- **Error propagation**: Chain `llvm::Error` with `.takeError()`, `errorToErrorCode()`, or `ExitOnError`.
- **Streaming output**: Use `raw_ostream` hierarchy for all output (stdout, files, buffers, sockets).
- **Threading**: Use `llvm::thread`, `ThreadPool` for worker pools, `llvm::sys::Mutex` for synchronization.
- **Process control**: Use `llvm::sys::Process` for environment, `llvm::sys::Program` for spawning, `sys::Signals` for handlers.

## Dependencies

### Internal
- **Error types**: Error.h, ErrorHandling.h
- **String types**: StringRef.h, Twine.h, StringMap.h
- **Numeric types**: APInt.h, APFloat.h, DynamicAPInt.h
- **Data structures**: SmallVector.h, SmallPtrSet.h, FoldingSet.h, IntervalMap.h, IntEqClasses.h
- **I/O**: raw_ostream.h, MemoryBuffer.h, FileOutputBuffer.h
- **File system**: FileSystem.h, Path.h, VirtualFileSystem.h
- **Process/System**: Process.h, Program.h, Signals.h, Threading.h, DynamicLibrary.h
- **Hashing**: Hash.h, MD5.h, SHA1.h, SHA256.h
- **Command line**: CommandLine.h, OptionStrCmp.h
- **Regex**: Regex.h (POSIX regex wrapper)
- **Format**: JSON.h, YAMLParser.h, YAMLTraits.h
- **Encoding**: ConvertUTF.h, Unicode.h, TextEncoding.h
- **Target**: TargetRegistry.h

### External
- Standard C++ library
- POSIX API (Unix/*)
- Windows API (Windows/*)
- zlib (for compression)
- Z3 solver (optional, compile-time detection)

<!-- MANUAL: -->
