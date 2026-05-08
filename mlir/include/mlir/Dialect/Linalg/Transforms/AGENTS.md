<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Linalg Transforms

## Purpose
Transformation implementations for the Linalg dialect: bufferization, hoisting, tiling interface implementations, subset insertion, and runtime op verification.

## Key Files
| File | Description |
|------|-------------|
| `Transforms.h` | Primary public transformation APIs (tiling, fusion, vectorization, etc.) |
| `AllInterfaces.h` | Convenience header to register all Linalg interface impls |
| `BufferizableOpInterfaceImpl.h` | Bufferization interface impls for linalg ops |
| `Hoisting.h` | Loop-invariant code motion and hoisting utilities |
| `TilingInterfaceImpl.h` | TilingInterface implementations for linalg ops |
| `SubsetInsertionOpInterfaceImpl.h` | Subset insertion interface impls |
| `RuntimeOpVerification.h` | Runtime op verification helpers |
| `ShardingInterfaceImpl.h` | Sharding interface impls for linalg ops |

## For AI Agents

### Working In This Directory
- `Transforms.h` is the main include for linalg transformation patterns and pipelines
- `AllInterfaces.h` registers all external model interfaces at once for tools/drivers

## Dependencies
- Depends on: Linalg IR, Bufferization IR, Tensor/MemRef dialects, Vector dialect

<!-- MANUAL: -->
