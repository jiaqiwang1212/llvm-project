<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tosa Utils

## Purpose
Utility functions for TOSA dialect: conversion helpers, quantization utilities, and shape inference utilities used during TOSA lowering.

## Key Files
| File | Description |
|------|-------------|
| `ConversionUtils.h` | Helpers for lowering TOSA ops to Linalg/Arith |
| `QuantUtils.h` | Quantization parameter helpers for TOSA quantized ops |
| `ShapeUtils.h` | Shape computation utilities for TOSA ops |

## For AI Agents

### Working In This Directory
- `ConversionUtils.h` provides `convertScalarToTensor` and similar lowering helpers
- `QuantUtils.h` computes TOSA quantization parameters from scale/zero-point values

## Dependencies
- Depends on: Tosa IR, Quant dialect, Arith dialect

<!-- MANUAL: -->
