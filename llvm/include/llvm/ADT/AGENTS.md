<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ADT (Abstract Data Types)

## Purpose

The ADT directory provides efficient, specialized data structures for compiler construction. These containers have no LLVM-specific dependencies—they are pure algorithmic abstractions used everywhere in LLVM for fast iteration, memory efficiency, and type safety. These structures are optimized for common compiler patterns like small collections, dense maps, and graph traversal.

Nearly every LLVM subsystem depends on at least one ADT; choosing the right container dramatically impacts compile-time performance.

## Key Files

| File | Description |
|------|-------------|
| `SmallVector.h` | Hybrid vector: stack storage for small counts, heap fallback for growth |
| `DenseMap.h` | Fast hash table with linear probing |
| `DenseSet.h` | Hash set built on DenseMap |
| `StringRef.h` | Non-owning string view (replaces std::string in many contexts) |
| `ArrayRef.h` | Non-owning array view for passing contiguous ranges |
| `StringMap.h` | Hash table optimized for string keys |
| `BitVector.h` | Dynamic bit vector with arbitrary size |
| `SmallBitVector.h` | Hybrid bit vector: inline storage for small sets |
| `APInt.h` | Arbitrary-precision integers |
| `APFloat.h` | Arbitrary-precision floating point |
| `APFixedPoint.h` | Fixed-point arithmetic |
| `DynamicAPInt.h` | APInt wrapper with variable bit width |
| `Twine.h` | Efficient string concatenation (lazy tree) |
| `Optional.h` | Optional/Maybe type (C++17 std::optional wrapper) |
| `PointerUnion.h` | Tagged union of pointer types |
| `PointerIntPair.h` | Combine pointer with small integer in one word |
| `PointerEmbeddedInt.h` | Embed integer directly in pointer storage |
| `ilist.h` | Intrusive doubly-linked list with O(1) removal |
| `ilist_node.h` | Node type for intrusive lists |
| `simple_ilist.h` | Simplified intrusive list (no sentinel overhead) |
| `MapVector.h` | Vector with fast map lookup (insertion order preserved) |
| `SetVector.h` | Set with iteration order matching insertion |
| `UniqueVector.h` | Vector that maintains uniqueness |
| `DenseMapInfo.h` | Hash traits for custom types in DenseMap |
| `GraphTraits.h` | Iterator interface for graph traversal |
| `BreadthFirstIterator.h` | BFS iterator for graphs |
| `DepthFirstIterator.h` | DFS iterator for graphs |
| `PostOrderIterator.h` | Post-order traversal |
| `SCCIterator.h` | Strongly connected components |
| `GenericDomTree.h` | Generic dominator tree (reusable for control flow and data flow) |
| `GenericLoopInfo.h` | Generic loop detection and nesting |
| `CycleInfo.h` | Cycle information (generalized loops) |
| `FoldingSet.h` | Hash-consed set with automatic deduplication |
| `EquivalenceClasses.h` | Union-find data structure |
| `IntEqClasses.h` | Equivalence classes for integers |
| `IntervalMap.h` | Efficient map for interval → value mappings |
| `IntervalTree.h` | Tree structure for interval queries |
| `PriorityQueue.h` | Binary heap priority queue |
| `PriorityWorklist.h` | Worklist with priority ordering |
| `SmallSet.h` | Set optimized for small cardinality |
| `SmallPtrSet.h` | Hash set for pointer-sized elements |
| `SmallString.h` | String with inline storage (SmallVector<char>) |
| `BitmaskEnum.h` | Type-safe bitset enums |
| `Bitfields.h` | Bitfield packing in integers |
| `CachedHashString.h` | String with cached hash value |
| `StringExtras.h` | String manipulation utilities |
| `StringSwitch.h` | Fast string matching (string → value switch) |
| `Hashing.h` | General-purpose hash function |
| `StableHashing.h` | Cross-platform stable hashing |
| `ScopedHashTable.h` | Hash table with scope-based rollback |
| `edit_distance.h` | String edit distance (Levenshtein) |
| `STLExtras.h` | Standard library compatibility and convenience functions |
| `iterator_range.h` | Range wrapper for iterator pairs |
| `iterator.h` | Iterator adapters and utilities |
| `Sequence.h` | Lazy numeric range generator |

## For AI Agents

### Working In This Directory

When choosing or modifying ADT structures:

1. **Prefer SmallVector** for stack-heavy workloads (most compiler passes)
2. **Prefer DenseMap** for dense key spaces (instruction-to-info mappings)
3. **Use StringRef** instead of std::string for parameters and non-owning references
4. **Use ArrayRef** for passing sequences without explicit size (bounds-safe)
5. **ilist** is used for doubly-linked structures where removal order is unknown
6. **Don't add floating-point fields** to small ADTs—packing matters for cache efficiency
7. **Test with realistic compiler inputs** before optimizing data structure layout
8. **Document the expected cardinality** of sets and maps (influences struct choice)

### Common Patterns

**SmallVector for dynamic lists:**
```cpp
SmallVector<Value*, 8> values; // Stack storage for 8 ptrs, heap fallback
values.push_back(v);
for (auto *val : values) { ... }
```

**DenseMap for fast lookups:**
```cpp
DenseMap<Instruction*, int> costMap;
costMap[I] = 10;
if (costMap.count(I)) { int c = costMap[I]; }
```

**StringRef for non-owning strings:**
```cpp
StringRef name = function->getName();
if (name.startswith("_")) { ... }
```

**ArrayRef for passing sequences:**
```cpp
void process(ArrayRef<Value*> values) {
  for (auto *v : values) { ... }
}
```

**PointerUnion for tagged pointers:**
```cpp
PointerUnion<Function*, BasicBlock*> entity = someFunc;
if (auto *F = entity.dyn_cast<Function*>()) { ... }
```

**Iterating with graph traits:**
```cpp
for (auto *succ : successors(BB)) { ... }
```

## Dependencies

### Internal

None—ADTs are pure algorithms with no LLVM-specific dependencies.

### External

- Standard library (C++17)
- No third-party dependencies

<!-- MANUAL: -->
