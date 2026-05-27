#include "llvm/MC/TargetRegistry.h"


// 初始化 Cpu0 的Machine Code 层，包括指令描述，寄存器信息，汇编语法等，最终会注册MCInstrInfo, MCRegisterInfo,
// MCAsmInfo等
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0TargetMC() {

}