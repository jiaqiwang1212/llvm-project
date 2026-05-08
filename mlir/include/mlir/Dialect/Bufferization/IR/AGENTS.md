<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bufferization IR

## Purpose
Core op and interface definitions for the Bufferization dialect. Includes the key interfaces (`BufferizableOpInterface`, `AllocationOpInterface`, `BufferDeallocationOpInterface`) and ops like `alloc_tensor`, `to_memref`, `to_tensor`.

## Key Files
| File | Description |
|------|-------------|
| `Bufferization.h` | Op class declarations |
| `BufferizationBase.td` | Dialect definition and base classes |
| `BufferizationOps.td` | ODS op definitions |
| `BufferizationEnums.td` | Enum definitions (layout maps, copy-on-write) |
| `BufferizableOpInterface.h` | Core bufferization interface declaration |
| `BufferizableOpInterface.td` | ODS interface definition |
| `AllocationOpInterface.h` | Allocation interface declaration |
| `AllocationOpInterface.td` | ODS allocation interface definition |
| `BufferDeallocationOpInterface.h` | Deallocation interface declaration |
| `BufferDeallocationOpInterface.td` | ODS deallocation interface definition |
| `BufferViewFlowOpInterface.h` | Buffer aliasing/view-flow interface declaration |
| `BufferViewFlowOpInterface.td` | ODS view-flow interface definition |
| `BufferizationTypeInterfaces.h` | Type-level bufferization interfaces |
| `BufferizationTypeInterfaces.td` | ODS type interface definitions |
| `DstBufferizableOpInterfaceImpl.h` | Default dest-style bufferization impls |
| `UnstructuredControlFlow.h` | Helpers for bufferizing unstructured control flow |

## For AI Agents

### Working In This Directory
- Implement `BufferizableOpInterface` for new ops via external model or direct mixin
- `DstBufferizableOpInterfaceImpl.h` provides default implementations for destination-passing-style ops

## Dependencies
- Depends on: MemRef dialect, Tensor dialect types

<!-- MANUAL: -->
