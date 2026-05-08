<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlowToSPIRV Conversion

## Purpose
Lowers ControlFlow dialect branch ops to SPIR-V control flow ops. Converts `cf.br` → `spirv.Branch`, `cf.cond_br` → `spirv.BranchConditional`.

## Key Files
| File | Description |
|------|-------------|
| `ControlFlowToSPIRV.cpp` | Conversion patterns from cf branch ops to spirv branch ops |
| `ControlFlowToSPIRVPass.cpp` | Pass definition and registration |

## For AI Agents

### Working In This Directory
- SPIR-V uses `spirv.Branch`/`spirv.BranchConditional` for unstructured branching within merge blocks.
- Block argument phi semantics must be converted to SPIR-V's variable-based phi (`spirv.Store` before branch).
- `cf.switch` is lowered via a chain of `spirv.BranchConditional` ops.

## Dependencies
- Source: `mlir/Dialect/ControlFlow`
- Target: `mlir/Dialect/SPIRV`

<!-- MANUAL: -->
