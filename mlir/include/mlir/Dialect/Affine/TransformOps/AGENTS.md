<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Affine TransformOps

## Purpose
Transform dialect extension ops that target the Affine dialect. Enables script-driven application of affine transformations (tiling, loop interchange, etc.) via the Transform IR.

## Key Files
| File | Description |
|------|-------------|
| `AffineTransformOps.h` | Transform op declarations |
| `AffineTransformOps.td` | ODS definitions for affine transform ops |

## For AI Agents

### Working In This Directory
- Edit `.td` file to add new transform ops; register them in the extension's `registerTransformExtension`
- Transform ops must implement `TransformOpInterface`

### Common Patterns
- Op names follow `transform.affine.*` convention
- Each op takes Transform handles (values of transform type) as operands

## Dependencies
- Depends on: Transform dialect IR, Affine dialect IR

<!-- MANUAL: -->
