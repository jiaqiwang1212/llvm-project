<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ReconcileUnrealizedCasts

## Purpose
Implements the ReconcileUnrealizedCasts pass. Eliminates `unrealized_conversion_cast` ops that remain after dialect conversion by folding round-trip cast pairs and failing on any that cannot be removed.

## Key Files
| File | Description |
|------|-------------|
| `ReconcileUnrealizedCasts.cpp` | Pattern to fold and eliminate unrealized cast chains |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- This pass is a cleanup step run after one or more dialect conversion passes
- It folds `unrealized_conversion_cast(unrealized_conversion_cast(x))` when types round-trip
- Any cast that cannot be resolved causes the pass to fail, indicating an incomplete lowering

### Common Patterns
- The fold pattern checks that the input type of the outer cast equals the output type of the inner cast
- Used as the final step in multi-stage lowering pipelines to verify completeness
- No `TypeConverter` needed; operates purely on op structure

## Dependencies
- Headers: `include/mlir/Conversion/ReconcileUnrealizedCasts/`
- Source/Target: Any dialect — this pass is dialect-agnostic
- Related: `mlir/Transforms/DialectConversion.h` (where the cast op is defined)

<!-- MANUAL: -->
