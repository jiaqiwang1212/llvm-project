<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LLVMIR Transforms

## Purpose
Transformation passes that operate on LLVM IR-level MLIR: adding COMDATs, legalization for export to LLVM backend, NVVM optimization, DI expression rewriting, inliner interface, and C wrapper generation.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `AddComdats.h` | Pass to add COMDAT sections for linkage |
| `DIExpressionLegalization.h` | Debug info expression legalization pass |
| `DIExpressionRewriter.h` | Debug info expression rewrite utilities |
| `InlinerInterfaceImpl.h` | Inliner interface implementation for LLVM ops |
| `LegalizeForExport.h` | Legalization pass before exporting to LLVM backend |
| `OptimizeForNVVM.h` | NVVM-specific optimization pass |
| `RequestCWrappers.h` | Pass to add C wrapper function annotations |
| `UseDefaultVisibilityPass.h` | Pass to normalize symbol visibility |

## For AI Agents

### Working In This Directory
- `LegalizeForExport.h` must run before calling `translateModuleToLLVMIR`
- `InlinerInterfaceImpl.h` must be registered to inline `llvm.call` ops

## Dependencies
- Depends on: LLVMIR dialect IR

<!-- MANUAL: -->
