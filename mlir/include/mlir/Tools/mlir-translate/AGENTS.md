<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/mlir-translate/

## Purpose
Entry-point headers for `mlir-translate`-style translation tools. Provides `MlirTranslateMain()` and the `Translation` registration infrastructure for registering import/export translation functions invoked by the tool.

## Key Files
| File | Description |
|------|-------------|
| `MlirTranslateMain.h` | `MlirTranslateMain()` — main entry point for translation driver tools |
| `Translation.h` | `TranslateFromMLIRRegistration`, `TranslateToMLIRRegistration`, `TranslateRegistration` — translation registration API |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Use `TranslateFromMLIRRegistration` to register a function that converts MLIR → external format.
- Use `TranslateToMLIRRegistration` to register a function that converts external format → MLIR.
- Both take a string key (e.g., `"export-llvmir"`), description, translation function, and dialect registration callback.
- Call `MlirTranslateMain(argc, argv, registry)` in the tool's `main()`.

### Common Patterns
- `static TranslateFromMLIRRegistration reg("my-export", "Export to Foo", translateFn, registerDialectsFn);`

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext, DialectRegistry)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/` (SourceMgr, raw_ostream)

<!-- MANUAL: -->
