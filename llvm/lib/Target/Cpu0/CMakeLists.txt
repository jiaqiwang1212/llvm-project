add_llvm_component_group(Cpu0 HAS_JIT)

set(LLVM_TARGET_DEFINITIONS Cpu0.td)
tablegen(LLVM Cpu0GenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(Cpu0CommonTableGenTarget) # 添加这行代码tablegen才会生成文件

add_llvm_target(Cpu0CodeGen
    Cpu0TargetMachine.cpp
    Cpu0TargetObjectFile.cpp
    Cpu0Subtarget.cpp

    LINK_COMPONENTS
    CodeGen
    CodeGenTypes
    Core
    Cpu0Desc
    Cpu0Info
    MC
    Support
    Target
    TargetParser

    ADD_TO_COMPONENT
    Cpu0
)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
