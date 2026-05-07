<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AVR Backend

## Purpose
Compiles LLVM IR to Atmel AVR 8-bit microcontroller machine code. Targets Arduino and embedded systems using AVR devices (ATmega, ATtiny, etc.).

## Key Files
| File | Description |
|------|-------------|
| `AVR.td` | Architecture features and instruction definitions |
| `AVRISelLowering.cpp` | DAG lowering, calling conventions |
| `AVRISelDAGToDAG.cpp` | Instruction selection patterns |
| `AVRInstrInfo.cpp` | Instruction metadata |
| `AVRFrameLowering.cpp` | Stack frame and prologue/epilogue |
| `AVRAsmPrinter.cpp` | Assembly output |
| `AVRDevices.td` | Device-specific configurations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `MCTargetDesc/` | MC layer: encoding, register info |
| `TargetInfo/` | Target registration |
| `AsmParser/` | Assembly parsing |
| `Disassembler/` | Machine code decoding |

## For AI Agents

### Working In This Directory
- Edit `.td` files for instruction changes, rebuild with `llvm-tblgen`
- Modify `AVRISelLowering.cpp` for calling conventions and custom lowering
- Update `AVRFrameLowering.cpp` for stack/prologue handling
- Device variants in `AVRDevices.td` control feature set

## Dependencies

### Internal
- `lib/CodeGen/` — Core code generation
- `lib/MC/` — Machine code layer

### External
- Atmel AVR ISA documentation
- Arduino/avr-gcc toolchain

