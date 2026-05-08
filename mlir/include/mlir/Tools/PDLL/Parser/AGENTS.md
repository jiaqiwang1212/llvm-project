<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/PDLL/Parser/

## Purpose
PDLL source parser and code-completion interface. Parses `.pdll` source files into the PDLL AST and provides code-completion callbacks used by the PDLL LSP language server.

## Key Files
| File | Description |
|------|-------------|
| `Parser.h` | `parsePDLL()` — parse a PDLL source file into an AST module |
| `CodeComplete.h` | `CodeCompleteContext` — callback interface for providing PDLL code completions |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `parsePDLL(sourceMgr, astContext, odsContext, codeCompleteContext)` is the main entry point.
- `odsContext` provides op definitions loaded from `.td` files for type checking during parse.
- `codeCompleteContext` is optional; when provided, the parser emits completion items at the cursor.

### Common Patterns
- Parse: `ast::Module *module = parsePDLL(sourceMgr, astCtx, odsCtx, /*codeComplete=*/nullptr);`
- With completions: implement `CodeCompleteContext` and pass to enable LSP code completion.

## Dependencies

### Internal
- `mlir/Tools/PDLL/AST/` (AST nodes and context)
- `mlir/Tools/PDLL/ODS/` (ODS context for op type resolution)

### External
- `llvm/Support/SourceMgr.h`

<!-- MANUAL: -->
