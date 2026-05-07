<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CodeGenTypes

## Purpose
Lowers LLVM IR types to machine value types (EVT/MVT) for code generation. Provides target-specific type lowering and layout information for backend compilation.

## Key Files
| File | Description |
|------|-------------|
| `LowLevelType.cpp` | Low-level type representation for code generation |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- LLVM IR to EVT/MVT type mapping
- Target-specific type lowering
- Data layout and alignment computation
- Integration with CodeGen/ for instruction selection

## Dependencies

### Internal
- `llvm/lib/IR/` — IR types and representation
- `llvm/lib/CodeGen/` — backend code generation
- `llvm/lib/Target/` — target-specific lowering rules

<!-- MANUAL: -->
