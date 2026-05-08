<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlowToSCF Conversion

## Purpose
Raises unstructured ControlFlow dialect ops to structured SCF dialect ops. Converts `cf.br`/`cf.cond_br` CFG patterns back to `scf.if`/`scf.while` where the structure is recoverable — the reverse direction of SCFToControlFlow.

## Key Files
| File | Description |
|------|-------------|
| `ControlFlowToSCF.cpp` | Structural recovery patterns: CFG → structured control flow |

## For AI Agents

### Working In This Directory
- This is a structurization pass — it identifies CFG patterns that correspond to structured constructs and rewrites them.
- Common patterns detected: if-then-else (diamond CFG), while loops (back edge with single entry).
- Not all CFGs can be structurized; irreducible control flow is not handled.
- Useful when importing from LLVM IR or other CFG-based formats and wanting to recover structured form.

## Dependencies
- Source: `mlir/Dialect/ControlFlow`
- Target: `mlir/Dialect/SCF`

<!-- MANUAL: -->
