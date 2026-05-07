<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Orc

## Purpose
Modern ORC JIT v2 (Optimized Routines for Compilation) — the recommended JIT framework for LLVM. Provides a flexible, composable stack-based architecture for JIT compilation with features like lazy compilation, speculation, and advanced linking. Replaces legacy MCJIT.

## Key Files
| File | Description |
|------|-------------|
| `LLJIT.cpp` | High-level JIT API (LLJIT class) |
| `ExecutionSession.cpp` | Core execution session management |
| `Core.cpp` | Core JIT APIs and symbol resolution |
| `Layer.cpp` | Base layer abstraction |
| `IRCompileLayer.cpp` | Layer for IR compilation to object code |
| `IRTransformLayer.cpp` | Layer for IR transformations (optimizations) |
| `ObjectLinkingLayer.cpp` | Low-level object linking layer |
| `LinkGraphLayer.cpp` | Link graph abstraction layer |
| `LinkGraphLinkingLayer.cpp` | Linking using JITLink |
| `ObjectTransformLayer.cpp` | Object file transformation layer |
| `CompileOnDemandLayer.cpp` | Lazy compilation on demand |
| `LazyObjectLinkingLayer.cpp` | Lazy object linking |
| `LazyReexports.cpp` | Lazy re-export handling |
| `EHFrameRegistrationPlugin.cpp` | Exception handling frame registration |
| `UnwindInfoRegistrationPlugin.cpp` | Unwind info registration for debugging |
| `JITDylib.cpp` | Dynamic library abstraction in JIT |
| `ThreadSafeModule.cpp` | Thread-safe IR module wrapper |
| `ObjectFileInterface.cpp` | Object file interface abstraction |
| `ExecutionUtils.cpp` | Execution utilities and helper functions |
| `CompileUtils.cpp` | Compilation utilities |
| `IndirectionUtils.cpp` | Indirection (stub) utilities |
| `JITLinkRedirectableSymbolManager.cpp` | Symbol redirection management |
| `JITLinkReentryTrampolines.cpp` | Reentry trampoline generation |
| `ELFNixPlatform.cpp` | ELF platform support (Linux, Unix) |
| `MachOPlatform.cpp` | MachO platform support (macOS) |
| `COFFPlatform.cpp` | COFF platform support (Windows) |
| `COFF.cpp` | COFF-specific implementation |
| `MachO.cpp` | MachO-specific implementation |
| `AbsoluteSymbols.cpp` | Absolute symbol handling |
| `JITTargetMachineBuilder.cpp` | Target machine builder for JIT |
| `ExecutorProcessControl.cpp` | Out-of-process execution control |
| `SelfExecutorProcessControl.cpp` | In-process execution control |
| `SimpleRemoteEPC.cpp` | Simple remote executor protocol |
| `EPCDynamicLibrarySearchGenerator.cpp` | Dynamic library symbol generation |
| `EPCGenericDylibManager.cpp` | Generic dynamic library manager |
| `EPCGenericJITLinkMemoryManager.cpp` | JIT Link memory management |
| `EPCGenericRTDyldMemoryManager.cpp` | RuntimeDyld-style memory management |
| `EPCIndirectionUtils.cpp` | Indirection utilities for out-of-process |
| `InProcessMemoryAccess.cpp` | In-process memory access |
| `MemoryMapper.cpp` | Abstract memory mapping |
| `MapperJITLinkMemoryManager.cpp` | JIT Link memory manager with mapper |
| `SimpleRemoteMemoryMapper.cpp` | Simple remote memory mapper |
| `LoadLinkableFile.cpp` | Utilities for loading object files |
| `LookupAndRecordAddrs.cpp` | Symbol lookup and address recording |
| `Mangling.cpp` | Symbol name mangling utilities |
| `OrcABISupport.cpp` | ABI-specific support |
| `RTDyldObjectLinkingLayer.cpp` | RuntimeDyld-based object linking |
| `ReOptimizeLayer.cpp` | Re-optimization layer for performance tuning |
| `SpeculateAnalyses.cpp` | Speculative compilation analysis |
| `Speculation.cpp` | Speculative compilation implementation |
| `TaskDispatch.cpp` | Task dispatch and work queue management |
| `RedirectionManager.cpp` | Symbol redirection management |
| `DebugUtils.cpp` | Debugging utilities |
| `BacktraceTools.cpp` | Backtrace support for JIT code |
| `SectCreate.cpp` | Section creation utilities |
| `OrcV2CBindings.cpp` | C API bindings for ORC v2 |

## For AI Agents

### Working In This Directory
- **LLJIT.cpp** is the high-level API; most users interact through LLJIT.
- **ExecutionSession.cpp** manages the overall JIT state; changes here affect all clients.
- Layers are composable; understand the layer abstraction before adding new layers.
- Platform files (ELFNixPlatform, MachOPlatform, COFFPlatform) handle platform-specific initialization; changes must be tested on target platforms.
- Exception handling and debugging (EHFrameRegistrationPlugin, UnwindInfoRegistrationPlugin, DebugUtils) are critical for reliable execution.
- Out-of-process execution (ExecutorProcessControl, EPCDynamicLibrarySearchGenerator) is advanced; coordinate with system infrastructure.
- Test all changes with LLJIT and simple IR modules before claiming success.

### Dependencies

#### Internal
- Depends on `../JITLink/` (low-level linking)
- Depends on `llvm/lib/IR/` (IR modules and functions)
- Depends on `llvm/lib/CodeGen/` (code generation)
- Depends on `llvm/lib/Target/` (target machine info)
- Depends on `llvm/lib/Object/` (object file parsing)
- Used by LLVM tools (lli) and external JIT clients

<!-- MANUAL: -->
