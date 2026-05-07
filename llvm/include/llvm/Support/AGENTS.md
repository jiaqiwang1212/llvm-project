<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Support

## Purpose

The Support directory provides low-level compiler and platform utilities that almost everything in LLVM depends on. This includes error handling abstractions (Error, ErrorOr, Expected), I/O primitives (raw_ostream, MemoryBuffer), type casting utilities (isa<>, cast<>, dyn_cast<>), debugging macros, file system access, command-line parsing, and platform abstraction (Host, Process, ThreadPool).

Support is deliberately separate from IR and ADT to keep core data structures lightweight and reusable. No circular dependencies: Support depends only on standard library and platform headers.

## Key Files

| File | Description |
|------|-------------|
| `Error.h` | Modern error handling with stack trace context |
| `ErrorOr.h` | Value-or-error type (superceded by Expected) |
| `Expected.h` | Template wrapping value or error (preferred modern approach) |
| `Casting.h` | Type casting utilities: isa<>, cast<>, dyn_cast<> |
| `raw_ostream.h` | Fast formatted output (stdout, stderr, files, buffers) |
| `MemoryBuffer.h` | Mmap-based file reading |
| `FileSystem.h` | Cross-platform file system operations (llvm::sys::fs::*) |
| `Path.h` | Path manipulation (llvm::sys::path::*) |
| `Process.h` | Process information (exit codes, CPU count, etc.) |
| `Host.h` | Host platform information (CPU arch, endianness) |
| `CommandLine.h` | Command-line argument parsing |
| `Debug.h` | Debug macro infrastructure (LLVM_DEBUG, dbgs()) |
| `DebugCounter.h` | Per-pass debug counters for bisecting failures |
| `Timer.h` | Elapsed time measurement |
| `Allocator.h` | Bump pointer allocator for arena allocation |
| `BumpPtrAllocator.h` | Efficient allocation pattern for temp objects |
| `Regex.h` | Regular expression matching |
| `JSON.h` | JSON parsing and generation |
| `YAMLParser.h` | YAML parsing |
| `YAMLTraits.h` | YAML serialization traits |
| `TargetSelect.h` | Loading target backends (InitializeAllTargets, etc.) |
| `TargetRegistry.h` | Registry for target backends |
| `DynamicLibrary.h` | Load shared libraries at runtime |
| `ManagedStatic.h` | Global static initialization with cleanup |
| `Threading.h` | Thread management primitives |
| `ThreadPool.h` | Concurrent task execution |
| `Mutex.h` | Mutual exclusion lock |
| `RWMutex.h` | Reader-writer lock |
| `CrashRecoveryContext.h` | Exception-safe recovery from crashes |
| `Signals.h` | Signal handlers for cleanup |
| `PrettyStackTrace.h` | Symbolic stack traces on crash |
| `Watchdog.h` | Timeout-based process termination |
| `LockFileManager.h` | File-based locking for concurrent access |
| `MemAlloc.h` | Safe memory allocation (checked_malloc, etc.) |
| `Memory.h` | Virtual memory operations |
| `DataExtractor.h` | Binary data reading with endianness handling |
| `BinaryStreamReader.h` | Reader for binary streams |
| `BinaryStreamWriter.h` | Writer for binary streams |
| `Compression.h` | Zlib/Zstd compression |
| `CRC.h` | CRC checksum computation |
| `MD5.h` | MD5 hashing |
| `SHA1.h` | SHA-1 hashing |
| `SHA256.h` | SHA-256 hashing |
| `BLAKE3.h` | BLAKE3 hashing |
| `Hash.h` | Generic hashing utilities |
| `Hashing.h` | Hash function implementation (see ADT/) |
| `Format.h` | Format string support (printf-like) |
| `FormatVariadic.h` | Variadic format strings |
| `ConvertUTF.h` | UTF-8/UTF-16 conversion |
| `Unicode.h` | Unicode character classification |
| `StringSaver.h` | String interning/deduplication |
| `InitLLVM.h` | RAII initialization of LLVM subsystems |
| `Locale.h` | Locale-aware string operations |
| `WithColor.h` | Colored terminal output |
| `ScopedPrinter.h` | Pretty-printing of nested structures |
| `Printable.h` | Value type for operator<< customization |
| `KnownBits.h` | Bit-level value range analysis |
| `KnownFPClass.h` | Floating-point class tracking |
| `MathExtras.h` | Bit manipulation (countLeadingZeros, etc.) |
| `Alignment.h` | Alignment value (power-of-2 validation) |
| `TypeSize.h` | Scalable type size representation |
| `InstructionCost.h` | Cost model for instructions |
| `DivisionByConstantInfo.h` | Magic constants for division optimization |
| `Endian.h` | Endianness-aware byte swapping |
| `SwapByteOrder.h` | Byte order conversion |
| `NativeFormatting.h` | Platform-native number formatting |
| `LEB128.h` | Little-endian base-128 variable-length encoding |
| `Parallel.h` | Parallel for_each and parallel sort |
| `RandomNumberGenerator.h` | Seeded deterministic RNG |
| `VersionTuple.h` | Version number comparison |
| `SpecialCaseList.h` | Pattern-based allow/deny lists |
| `Discriminator.h` | Debug discriminator encoding |
| `VirtualFileSystem.h` | In-memory file system overlay |

## For AI Agents

### Working In This Directory

When using or modifying Support utilities:

1. **Prefer Expected<T>** over ErrorOr for new code (more ergonomic)
2. **Use raw_ostream** instead of std::cout or std::cerr (respects LLVM output redirection)
3. **Use llvm::dbgs()** for debug output, guarded by LLVM_DEBUG() macros for zero cost when disabled
4. **Use SmallVector from ADT/** for temporary collections passed to Support APIs
5. **isa<>/cast<>/dyn_cast<>** require explicit specialization; document RTTI behavior for custom types
6. **Don't allocate in tight loops**—use Allocator.h and reset() between iterations
7. **File I/O via MemoryBuffer** (mmap) is faster than manual reading
8. **Target selection** is lazy—use TargetRegistry without calling InitializeAllTargets unless needed
9. **Threading** is not always enabled (check LLVM_ENABLE_THREADS); use ManagedStatic for thread-safe globals

### Common Patterns

**Error handling with Expected:**
```cpp
Expected<std::unique_ptr<Module>> loadModule(StringRef path) {
  auto bufOrErr = MemoryBuffer::getFile(path);
  if (!bufOrErr) return bufOrErr.takeError();
  // ... parse module
  return std::move(module);
}

if (auto M = loadModule("file.ll")) {
  // Use *M
} else {
  // Handle Error
}
```

**Fast I/O:**
```cpp
raw_ostream &out = llvm::outs();
out << "Value: " << value << "\n";
out.flush();
```

**Type checking and casting:**
```cpp
if (isa<Instruction>(val)) {
  auto *I = cast<Instruction>(val);
  // I is guaranteed non-null
}
if (auto *I = dyn_cast<Instruction>(val)) {
  // I is non-null only if val is Instruction
}
```

**Debugging output:**
```cpp
LLVM_DEBUG(dbgs() << "Processing " << I << "\n");
```

**Memory-efficient allocation:**
```cpp
BumpPtrAllocator allocator;
auto *ptr = allocator.Allocate<MyType>();
new (ptr) MyType();
// No individual deallocate; reset() clears all
```

**File reading:**
```cpp
auto buf = MemoryBuffer::getFile("input.txt");
if (!buf) { /* handle error */ }
StringRef content = buf->get()->getBuffer();
```

## Dependencies

### Internal

- `ADT/` — SmallVector, StringRef, DenseMap for collections
- `Config/` — Build-time settings (threading enabled, target list)

### External

- Platform headers (unistd.h, windows.h)
- Standard library (C++17)
- Optional: zlib, zstd (compression)

<!-- MANUAL: -->
