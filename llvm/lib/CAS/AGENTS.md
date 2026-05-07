<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CAS

## Purpose

Content-addressable storage (CAS) implementation for LLVM. Provides persistent, distributed cache of build artifacts indexed by content hash. Enables incremental builds and artifact sharing across compilation jobs.

## Key Files

| File | Description |
|------|-------------|
| `ActionCache.cpp` | Per-action artifact caching by content hash |
| `ActionCaches.cpp` | Multiple action cache management |
| `BuiltinCAS.cpp` | Built-in (in-process) CAS implementation |
| `BuiltinCAS.h` | Built-in CAS interface |
| `BuiltinObjectHasher.cpp` | Object content hashing |
| `BuiltinUnifiedCASDatabases.cpp` | Unified CAS database management |
| `CASNodeSchema.cpp` | Node schema and versioning |
| `DatabaseFile.cpp` | CAS persistent storage layer |
| `DatabaseFile.h` | Database file interface |
| `InMemoryCAS.cpp` | Ephemeral in-memory CAS |
| `MappedFileRegionArena.cpp` | Memory-mapped file arena allocator |
| `NamedValuesSchema.cpp` | Named value schema support |
| `ObjectStore.cpp` | Object storage and retrieval |
| `OnDiskCAS.cpp` | On-disk persistent CAS |
| `OnDiskCASLogger.cpp` | CAS operation logging |
| `OnDiskCommon.cpp` | Shared on-disk routines |
| `OnDiskCommon.h` | Common on-disk structures |
| `OnDiskDataAllocator.cpp` | On-disk data allocation |
| `OnDiskGraphDB.cpp` | On-disk graph database |
| `OnDiskKeyValueDB.cpp` | On-disk key-value store |
| `OnDiskTrieRawHashMap.cpp` | Trie-based hash map for on-disk storage |
| `UnifiedOnDiskCache.cpp` | Unified on-disk caching |

## For AI Agents

### Working In This Directory

1. Understand content-addressable storage concepts and hash-based indexing
2. Know how build artifacts map to content hashes
3. Test CAS operations: store, retrieve, validate, garbage collection
4. Handle concurrent access to shared cache
5. Verify data consistency and corruption detection
6. Test cache efficiency and eviction policies

### Key Patterns

- Content hash (SHA256 or similar) serves as primary key
- Artifacts stored once, referenced by hash
- ActionCache maps build actions to cached results
- On-disk storage provides persistence across builds
- In-memory CAS for ephemeral caching within a build

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM IR
- Used by: Build system, ThinLTO, distributed build systems

<!-- MANUAL: -->
