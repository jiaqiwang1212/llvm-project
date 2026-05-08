<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPU IR

## Purpose
Core op, interface, and attribute definitions for the GPU dialect. Includes launch ops, thread/block index ops, memory hierarchy ops, shuffle ops, and compilation attribute interfaces.

## Key Files
| File | Description |
|------|-------------|
| `GPUDialect.h` | Dialect class declaration |
| `GPUBase.td` | Dialect definition and base classes |
| `GPUOps.td` | ODS op definitions |
| `GPUDeviceMappingAttr.td` | Device mapping attribute definitions |
| `ParallelLoopMapperAttr.td` | Parallel loop mapper attribute |
| `CompilationAttrs.td` | GPU binary/module compilation attributes |
| `CompilationAttrInterfaces.td` | Compilation attribute interfaces |
| `CompilationInterfaces.h` | C++ compilation interface declarations |
| `ValueBoundsOpInterfaceImpl.h` | ValueBounds impls for GPU index ops |

## For AI Agents

### Working In This Directory
- `gpu.module` is a nested symbol table; `gpu.func` inside it is the kernel function
- Compilation attributes record embedded binary blobs for serialized GPU code

## Dependencies
- Depends on: MemRef dialect, LLVMIR types, Func dialect

<!-- MANUAL: -->
