<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRef Transforms

## Purpose
Transformation passes for the MemRef dialect: subview composition, expand/collapse shape, fold memref aliases, runtime op verification, and bufferization/view-flow interface implementations.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |
| `ComposeSubView.h` | Subview composition patterns |
| `AllocationOpInterfaceImpl.h` | Allocation interface impls for memref alloc ops |
| `BufferViewFlowOpInterfaceImpl.h` | Buffer view-flow interface impls for memref ops |
| `RuntimeOpVerification.h` | Runtime bounds-checking verification helpers |

## For AI Agents

### Working In This Directory
- `ComposeSubView.h` provides patterns for folding chains of subview ops
- Implementations live in `lib/Dialect/MemRef/Transforms/`

## Dependencies
- Depends on: MemRef IR, Bufferization IR

<!-- MANUAL: -->
