<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DTLTO

## Purpose

Distributed ThinLTO implementation. Enables parallel link-time optimization across multiple machines for faster builds in large projects.

## Key Files

| File | Description |
|------|-------------|
| `DTLTO.cpp` | Distributed ThinLTO orchestration |

## For AI Agents

### Working In This Directory

1. Understand ThinLTO fundamentals (lib/LTO/ThinLTOCodeGenerator.cpp)
2. Know distributed build communication and work distribution
3. Test multi-machine builds with DTLTO
4. Verify work partitioning and load balancing
5. Handle network failures and partial completion
6. Test cache locality and artifact management
7. Benchmark speedup relative to single-machine ThinLTO

### Key Patterns

- ThinLTO units distributed across machines
- Each worker optimizes and codegen independently
- Results aggregated at link time
- Work stealing and load balancing for efficiency
- Fallback to single-machine ThinLTO on distribution failure

## Dependencies

### Internal
- Depends on: LLVM LTO (lib/LTO), LLVM IR, distributed system libraries
- Used by: Build systems supporting distributed ThinLTO (e.g., DistributedThinLTO mode)

<!-- MANUAL: -->
