<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Frontend/Offloading

## Purpose

Provides GPU and accelerator offloading metadata infrastructure for CUDA, HIP, OpenMP target offload, and other accelerator programming models. Handles target device information, offloading region metadata, and device code bundling.

## Key Files

| File | Description |
|------|-------------|
| `OffloadWrapper.cpp` | Wraps device code for multi-device offloading |
| `PropertySet.cpp` | Offloading property set representation and management |
| `Utility.cpp` | Utility functions for offloading metadata handling |

## For AI Agents

### Working In This Directory

1. Understand device-side and host-side compilation separation
2. Familiarize with device triple formats (nvptx64-nvidia-cuda, amdgcn-amd-amdhsa, etc.)
3. Test multi-device bundling and code packaging
4. Verify device metadata is correctly embedded in host binaries
5. Check device feature detection and capability handling

### Key Patterns

- Offloading regions identified and wrapped during host compilation
- Device code compiled separately and bundled with host binary
- Metadata tracks device architecture, code paths, and linking requirements
- Runtime system unpacks and selects appropriate device code at execution

## Dependencies

### Internal
- Depends on: LLVM IR, CodeGen, Frontend utilities
- Used by: OpenMP offloading (lib/Frontend/OpenMP), CUDA/HIP frontends

<!-- MANUAL: -->
