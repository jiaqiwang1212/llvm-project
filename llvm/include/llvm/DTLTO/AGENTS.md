<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DTLTO

## Purpose

Distributed ThinLTO coordination and work distribution. Splits ThinLTO optimization work across multiple machines via network communication. Enables parallelism beyond local CPU cores for large-scale compilation (Google Blaze, high-end CI systems).

## Key Files

| File | Description |
|------|-------------|
| `DTLTO.h` | Distributed ThinLTO coordinator; work distribution, result aggregation (3 KB) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding distributed ThinLTO architecture
- Analyzing work distribution strategies
- Reviewing cross-machine coordination logic

**Implementation tasks:**
- Adding network transport backends (gRPC, REST)
- Implementing new work scheduling algorithms
- Enhancing result aggregation and fallback

### Common Patterns

1. **Work units**: ThinLTO imports/exports split into work items; distributed to available workers
2. **Remote execution**: Worker machine receives bitcode, runs ThinLTO optimization pass, returns optimized bitcode
3. **Result merging**: Coordinator collects optimized results from workers; links into final binary
4. **Fallback**: If distributed compilation unavailable, falls back to local ThinLTO
5. **Caching**: Intermediate optimization results cached to avoid redundant work

### Distributed ThinLTO Architecture

```
[Local machine]
  - Parse bitcode, identify work units (imports/exports)
  - Distribute to worker pool
  ├-> [Worker 1] optimize CU 1
  ├-> [Worker 2] optimize CU 2
  └-> [Worker 3] optimize CU 3
  - Collect results, merge, link
  -> Final binary
```

### Configuration

- **Worker pool**: Network addresses of available workers (gRPC, HTTP endpoints)
- **Timeout**: Per-work-unit timeout; resubmit if worker unresponsive
- **Fallback strategy**: Local ThinLTO if distribution unavailable

### Important Notes

- **ThinLTO prerequisite**: Requires ThinLTO enabled (`-flto=thin`); not applicable to FullLTO
- **Network overhead**: Benefit only if optimization cost > network latency; typically beneficial for large projects (millions of LOC)
- **Determinism**: Results must be bit-identical across local/distributed paths (critical for build reproducibility)

## Dependencies

### Internal
- `llvm/Bitcode/` — BitcodeReader, BitcodeWriter for work unit I/O
- `llvm/IR/` — Module, Function for IR manipulation
- `llvm/LTO/` — ThinLTO optimizer
- `llvm/Support/` — Error, network utilities (if available)

### External
- gRPC or equivalent RPC framework (optional; for remote execution)

<!-- MANUAL: -->
