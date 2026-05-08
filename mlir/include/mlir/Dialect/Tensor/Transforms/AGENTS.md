<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tensor Transforms

## Purpose
Transformation passes and patterns for the Tensor dialect: bufferization interface implementations, subset insertion, runtime op verification, and tensor reshape/fold patterns.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |
| `TransformUtils.h` | Shared transformation utility functions |
| `BufferizableOpInterfaceImpl.h` | Bufferization interface impls for tensor ops |
| `SubsetInsertionOpInterfaceImpl.h` | Subset insertion interface impls |
| `RuntimeOpVerification.h` | Runtime bounds checking for tensor ops |

## For AI Agents

### Working In This Directory
- `Transforms.h` provides `populateTensorFoldIntoPackAndUnpackPatterns` and similar helpers
- Implementations live in `lib/Dialect/Tensor/Transforms/`

## Dependencies
- Depends on: Tensor IR, Bufferization IR, Linalg dialect

<!-- MANUAL: -->
