<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LTO

## Purpose
Implements Full LTO and ThinLTO whole-program optimization pipelines. Performs cross-module optimizations and code generation for Link-Time Optimization.

## Key Files
| File | Description |
|------|-------------|
| `LTO.cpp` | Main LTO driver and configuration |
| `LTOBackend.cpp` | Backend code generation integration |
| `LTOCodeGenerator.cpp` | Legacy Full LTO code generator |
| `LTOModule.cpp` | LTO module representation |
| `ThinLTOCodeGenerator.cpp` | ThinLTO code generator and parallel backend |
| `UpdateCompilerUsed.cpp` | Compiler-used metadata updates |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Full LTO vs ThinLTO pipeline design
- Cross-module inlining and optimization
- Parallel backend execution for ThinLTO
- Integration with CodeGen/ for machine code emission
- Bitcode vs IR-based optimization

## Dependencies

### Internal
- `llvm/lib/Linker/` — module linking
- `llvm/lib/CodeGen/` — backend code generation
- `llvm/lib/Transforms/` — optimization passes
- `llvm/lib/IR/` — IR representation

<!-- MANUAL: -->
