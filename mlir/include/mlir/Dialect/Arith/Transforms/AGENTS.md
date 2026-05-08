<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Arith Transforms

## Purpose
Transformation passes for the Arith dialect: narrow-type emulation, wide-integer emulation, bufferization interface implementations, and sharding interface implementations.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |
| `NarrowTypeEmulationConverter.h` | Type converter for narrow integer emulation |
| `WideIntEmulationConverter.h` | Type converter for wide integer emulation |
| `BufferizableOpInterfaceImpl.h` | Bufferization interface impls for arith ops |
| `BufferDeallocationOpInterfaceImpl.h` | Buffer deallocation interface impls |
| `BufferViewFlowOpInterfaceImpl.h` | Buffer view flow interface impls |
| `ShardingInterfaceImpl.h` | Sharding interface implementations |

## For AI Agents

### Working In This Directory
- Emulation converters are type converters used in dialect conversion passes
- Bufferization impls register with `registerArithBufferizableOpInterfaceExternalModels`

## Dependencies
- Depends on: Arith IR, Bufferization IR, Shard interfaces

<!-- MANUAL: -->
