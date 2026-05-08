<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRV Dialect

## Purpose
Represents SPIR-V (Standard Portable Intermediate Representation for Vulkan/OpenCL) in MLIR. Provides a near-complete mapping of SPIR-V instructions, types, decorations, and execution models for GPU compute and graphics shader compilation.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Interfaces/` | SPIRV image interfaces (see `Interfaces/AGENTS.md`) |
| `IR/` | Op, type, and attribute definitions (see `IR/AGENTS.md`) |
| `Linking/` | SPIR-V module linking utilities (see `Linking/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | Layout and utility helpers (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- SPIR-V ops are organized by instruction category in separate `.td` files
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- Op names follow `spirv.*` convention (e.g., `spirv.IAdd`, `spirv.Store`, `spirv.EntryPoint`)
- `spirv.module` is the top-level container with execution model and addressing model

## Dependencies
- Receives lowerings from: GPU, Arith, Math, MemRef, Func, SCF, ControlFlow, Tensor dialects

<!-- MANUAL: -->
