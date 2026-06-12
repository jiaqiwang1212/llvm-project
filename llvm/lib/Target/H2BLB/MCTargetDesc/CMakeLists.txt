add_llvm_component_library(LLVMH2BLBDesc
  H2BLBMCTargetDesc.cpp
  H2BLBMCAsmInfo.cpp

  LINK_COMPONENTS
  Support
  H2BLBInfo
  MC

  ADD_TO_COMPONENT
  H2BLB
  )
