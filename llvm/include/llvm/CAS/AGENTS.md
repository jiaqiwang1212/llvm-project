<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CAS

## Purpose
Content-addressable storage system for incremental builds and distributed caching. Provides storage backends, action caching, and artifact persistence.

## Key Files
| File | Description |
|------|-------------|
| `ObjectStore.h` | Content-addressed object storage |
| `ActionCache.h` | Cache compilation results by action |
| `BuiltinCASContext.h` | Default CAS implementation |
| `BuiltinObjectHasher.h` | Hash computation for objects |
| `BuiltinUnifiedCASDatabases.h` | Unified CAS database implementation |
| `CASID.h` | Content-addressed storage identifier |
| `CASNodeSchema.h` | Node schema definitions |
| `CASReference.h` | References to CAS objects |
| `FileOffset.h` | File offset tracking |
| `MappedFileRegionArena.h` | Memory-mapped file management |
| `NamedValuesSchema.h` | Named value schema definitions |
| `OnDiskCASLogger.h` | Logging for CAS operations |
| `OnDiskDataAllocator.h` | On-disk data allocation |
| `OnDiskGraphDB.h` | Graph database for CAS |
| `OnDiskKeyValueDB.h` | Key-value store backend |
| `OnDiskTrieRawHashMap.h` | Hash map using trie structure |
| `UnifiedOnDiskCache.h` | On-disk cache backend |
| `ValidationResult.h` | Validation results |

## For AI Agents

### Working In This Directory
CAS enables distributed and incremental compilation:
- **Content addressing**: Identify objects by content hash (not path)
- **Action caching**: Cache compilation results keyed by inputs
- **Distributed sharing**: Cache shared across machines
- **Graph storage**: Track dependencies between cached items

### Common Patterns
- Create ObjectStore for artifact management
- Use ActionCache to memoize compilation steps
- Content-based addressing for cache invalidation
- Distributed backend for team/CI systems

## Dependencies

### Internal
- `llvm/Support` (Error, StringRef, hashing)
- `llvm/ADT` (DenseMap, SmallVector)

<!-- MANUAL: -->
