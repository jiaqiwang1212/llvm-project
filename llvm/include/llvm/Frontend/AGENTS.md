<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend

## Purpose
Language-agnostic frontend support libraries for pragma handling, OpenMP codegen, and offloading metadata. Provides abstractions for multiple languages (C, C++, Fortran) to share common compilation infrastructure.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Atomic/` | Atomic operations support |
| `Debug/` | Debug information utilities |
| `Directive/` | Pragma/directive processing |
| `Driver/` | Compiler driver utilities |
| `HLSL/` | High-level shading language support |
| `Offloading/` | Offloading (GPU, accelerator) metadata |
| `OpenACC/` | OpenACC pragma support |
| `OpenMP/` | OpenMP pragma codegen and support |

## For AI Agents

### Working In This Directory
The Frontend directory serves as a shared resource for language frontends. Key workflows:
- **Pragma handling**: Directives/ provides infrastructure for parsing and processing pragmas (OpenMP, OpenACC, etc.)
- **Offloading compilation**: Offloading/ manages metadata for GPU/heterogeneous compilation
- **OpenMP codegen**: OpenMP/ contains lowering passes and runtime support

### Common Patterns
- Recursive pragma processing through AST visitors
- Target-agnostic metadata (reusable across backends)
- Integration with CodeGen for IR emission

## Dependencies

### Internal
- `llvm/IR` (AST/IR manipulation)
- `llvm/Support` (utilities)

<!-- MANUAL: -->
