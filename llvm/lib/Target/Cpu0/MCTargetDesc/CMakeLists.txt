add_llvm_component_library(LLVMCpu0Desc
  Cpu0MCTargetDesc.cpp

  LINK_COMPONENTS
  CodeGenTypes
  MC
  Cpu0Info
  Support
  TargetParser

  ADD_TO_COMPONENT
  Cpu0
  )
