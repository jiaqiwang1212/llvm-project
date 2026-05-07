<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend

## Purpose

Language-agnostic frontend support infrastructure. This directory groups subdirectories that provide shared utilities and code generation support for compiler frontends (clang, flang, etc.). The root directory contains CMakeLists.txt for managing the subdirectory build system.

## Subdirectories

- **Atomic/** - Atomic operation lowering helpers for frontends
- **Directive/** - OpenMP/OpenACC directive metadata and parsing support
- **Driver/** - Driver utility functions shared between clang and other frontends
- **HLSL/** - HLSL (High-Level Shading Language) IR utilities for DirectX
- **Offloading/** - GPU/accelerator offloading metadata (for CUDA, HIP, OpenMP target offload)
- **OpenACC/** - OpenACC directive support (early stage)
- **OpenMP/** - OpenMP code generation support

## For AI Agents

### Working In This Directory

This is an organizational directory. Do not add source files directly here. Instead:
1. Use appropriate subdirectories based on feature (OpenMP, Offloading, HLSL, etc.)
2. Read subdirectory AGENTS.md files for specific implementation guidance
3. Use CMakeLists.txt to manage subdirectory interdependencies

### Key Patterns

- Each subdirectory handles a distinct frontend concern
- Subdirectories can depend on each other but avoid circular dependencies
- Shared types and utilities typically live in the most general parent that needs them

## Dependencies

### Internal
- Depends on: LLVM IR (lib/IR)
- Used by: Frontends (clang, flang, etc.), backend code generation

<!-- MANUAL: -->
