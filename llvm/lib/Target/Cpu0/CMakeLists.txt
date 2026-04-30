add_llvm_component_group(Cpu0)

set(LLVM_TARGET_DEFINITIONS Cpu0Other.td)

tablegen(LLVM Cpu0GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM Cpu0GenCallingConv.inc -gen-callingconv)
tablegen(LLVM Cpu0GenCodeEmitter.inc -gen-emitter)
tablegen(LLVM Cpu0GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM Cpu0GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM Cpu0GenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM Cpu0GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM Cpu0GenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(Cpu0CommonTableGen)

add_llvm_target(Cpu0CodeGen
  Cpu0AnalyzeImmediate.cpp
  Cpu0AsmPrinter.cpp
  Cpu0FrameLowering.cpp
  Cpu0ISelDAGToDAG.cpp
  Cpu0InstrInfo.cpp
  Cpu0ISelLowering.cpp
  Cpu0MCInstLower.cpp
  Cpu0MachineFunction.cpp
  Cpu0RegisterInfo.cpp
  Cpu0SEFrameLowering.cpp
  Cpu0SEInstrInfo.cpp
  Cpu0SEISelDAGToDAG.cpp
  Cpu0SEISelLowering.cpp
  Cpu0SERegisterInfo.cpp
  Cpu0Subtarget.cpp
  Cpu0TargetMachine.cpp
  Cpu0TargetObjectFile.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
  Cpu0Desc
  Cpu0Info
  Cpu0AsmPrinter
  SelectionDAG
  Support
  Target
  TargetParser
  GlobalISel

  ADD_TO_COMPONENT
  Cpu0
)

add_subdirectory(InstPrinter)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
