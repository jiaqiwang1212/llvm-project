<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TargetParser

## Purpose

Parsing and normalization of target triple strings, CPU feature strings, and CPU names. Provides host detection and target triple parsing without any CodeGen dependency. Used by TargetRegistry to map user-specified target names to target machines, and by tools to understand target architectures.

## Key Files

| File | Purpose |
|------|---------|
| `Triple.h` | Parse and normalize target triple (arch-vendor-os-env format) |
| `SubtargetFeature.h` | Parse and manage CPU feature strings (e.g., "+avx", "-sse4.2") |
| `Host.h` | Detect host CPU and features at runtime |
| `TargetParser.h` | General target parsing utilities |
| `AArch64TargetParser.h` | AArch64-specific CPU names and features |
| `AArch64CPUFeatures.inc` | Generated AArch64 CPU database |
| `AArch64FeatPriorities.inc` | AArch64 feature priority definitions |
| `ARMTargetParser.h` | ARM-specific CPU names and features |
| `ARMTargetParserCommon.h` | Shared ARM/Thumb parsing logic |
| `ARMTargetParser.def` | ARM CPU and feature database |
| `X86TargetParser.h` | X86-specific CPU names and features |
| `X86TargetParser.def` | X86 CPU database (Pentium, Core, Skylake, etc.) |
| `RISCVTargetParser.h` | RISC-V-specific parsing |
| `RISCVISAInfo.h` | RISC-V ISA string parsing and normalization |
| `RISCVTargetParser.h` | RISC-V CPU names |
| `PPCTargetParser.h` | PowerPC-specific parsing |
| `PPCTargetParser.def` | PowerPC CPU database |
| `AVRTargetParser.h` | AVR-specific parsing |
| `LoongArchTargetParser.h` | LoongArch-specific parsing |
| `CSKYTargetParser.h` | CSKY-specific parsing |
| `XtensaTargetParser.h` | Xtensa-specific parsing |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

**Understand target triple format:**
- Format: `arch-vendor-os-env` (e.g., `x86_64-unknown-linux-gnu`, `aarch64-apple-darwin`)
- Triple::parse() normalizes strings into canonical form
- Triple provides accessor methods: `getArch()`, `getOS()`, `getVendor()`, `getEnvironment()`

**CPU feature strings:**
- Format: comma-separated list of `+feature` or `-feature`
- SubtargetFeature parses and compares features
- Features are target-specific (e.g., `+avx` for X86, `+neon` for ARM)

**Host detection:**
- Host::getHostCPUName() returns CPU name of build machine (e.g., "skylake")
- Host::getHostCPUFeatures() returns features of build machine
- Used for cross-compilation and JIT compilation targets

**Architecture-specific parsers:**
- X86TargetParser: CPU names like "skylake", "zen3"
- ARMTargetParser: CPU names like "cortex-a72", "cortex-m4"
- AArch64TargetParser: CPU names like "cortex-a76", "neoverse-n1"
- RISCVTargetParser: ISA string parsing (RV64IMC, etc.)
- Others: PowerPC, AVR, RISC-V, LoongArch, CSKY, Xtensa

### Common Patterns

**Parse a target triple:**
```cpp
Triple TT("x86_64-unknown-linux-gnu");
if (TT.getArch() == Triple::x86_64) { ... }
if (TT.getOS() == Triple::Linux) { ... }
```

**Normalize and handle triple:**
```cpp
std::string TripleStr = "x86_64-linux";
Triple TT(Triple::normalize(TripleStr));
// Now TT has canonical form with vendor, env filled in
```

**Parse CPU features:**
```cpp
SubtargetFeatures Features("base,+avx,+avx2,-sse4.2");
uint64_t Flags = Features.getFeatureBits(CPU, MCSubtargetInfo);
```

**Get host CPU:**
```cpp
std::string HostCPU = sys::getHostCPUName();
SubtargetFeatures HostFeatures;
sys::getHostCPUFeatures(HostFeatures);
```

**Arch-specific CPU parsing:**
```cpp
X86::CPUKind Kind = X86::parseCPUKind("skylake");
if (Kind != X86::CK_Unspecified) {
  std::string FeatureStr = X86::getCPUFeatures(Kind);
}
```

## Dependencies

### Internal
- `Support/` - string utilities, SmallVector, host detection
- No CodeGen, MC, or IR dependencies (standalone utility layer)

### Usage
- `Target/` - TargetRegistry uses Triple to map target names
- `MC/` - MCSubtargetInfo uses feature flags
- Tools (clang, llc, opt, llvm-mc) use Triple and SubtargetFeature for target selection

<!-- MANUAL: -->
