<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MIRParser

## Purpose

Parses machine IR (MIR) text format (.mir files) into machine function IR. Enables testing and debugging the CodeGen pipeline by:
- Starting compilation at arbitrary pipeline stages (e.g., `llc --start-before regalloc`)
- Writing test cases in human-readable machine IR
- Debugging specific passes without running the full pipeline

## Key Files

| File | Purpose |
|------|---------|
| `MIRParser.cpp` | Main parser driver, orchestrates lexer and AST construction |
| `MIParser.cpp` | Machine IR AST parser (~1800 lines) |
| `MILexer.cpp` | Lexical analyzer for .mir text format |
| `MILexer.h` | Lexer token and state definitions |

## For AI Agents

### Working In This Directory

This directory implements a single, focused tool: parsing .mir text format. Start here for:
- .mir file format grammar and semantics
- Lexical and syntactic analysis
- Converting text MIR into MachineFunction IR objects
- Error recovery and diagnostics

Simple three-stage pipeline: Lexer (text→tokens) → Parser (tokens→AST) → Construction (AST→MachineFunction).

### Common Patterns

**.mir File Format**:
```mir
--- |
  define i32 @foo(i32 %arg0) {
  bb.0:
    ret i32 %arg0
  }
...
name:            foo
alignment:       16
exposesReturnsTwice: false
callsUnwindables : false
hasOpaqueSPAdjustment : false
hasVAStart       : false
legalized        : false
regBankSelected  : false
selected         : false
failedISel       : false
tracksRegLiveness: true
registers:
  - { id: 0, class: gpr32 }
  - { id: 1, class: gpr32 }
body:             |
  bb.0:
    %1 = COPY %0
    $eax = MOV32rr %1
    RET 0, $eax
```

**Parser Workflow**:
1. Lexer tokenizes the text
2. Parser builds abstract syntax tree (AST) of function/block/instruction structure
3. Construction phase creates MachineFunction objects from AST

**Error Handling**: Parser provides good diagnostics for malformed .mir (line/column, expected vs. actual tokens).

## Dependencies

### Internal
- `../` — CodeGen core (MachineFunction, MachineInstr, MachineBasicBlock)
- `include/llvm/CodeGen/MachineValueType.h` — Register and type enumerations

### External
- `Support/SourceMgr.h` — Source location and error reporting
- `Support/SMLoc.h` — Source location info for diagnostics

<!-- MANUAL: -->
