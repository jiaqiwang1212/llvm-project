<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ShardToMPI

## Purpose
Implements the Shard to MPI lowering pass. Converts mesh sharding dialect operations (collective communications, data redistribution across mesh dimensions) into `mpi` dialect calls for distributed execution.

## Key Files
| File | Description |
|------|-------------|
| `ShardToMPI.cpp` | Conversion patterns from shard/mesh ops to MPI dialect ops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateShardToMPIConversionPatterns()`
- Mesh topology and shard assignments guide the generation of point-to-point or collective MPI calls

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- Collective ops (allreduce, allgather) map to corresponding `mpi.*` collective ops

## Dependencies
- Headers: `include/mlir/Conversion/ShardToMPI/`
- Source dialect: `lib/Dialect/Mesh/`
- Target dialect: `lib/Dialect/MPI/`

<!-- MANUAL: -->
