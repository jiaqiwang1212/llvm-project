#include "TargetInfo/Cpu0TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheCpu0Target() {
  static Target TheMipsTarget;
  return TheMipsTarget;
}

// 注册 Cpu0 target 到llvm的全局Target中，为了llvm可以识别 Cpu0
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeCpu0TargetInfo() {
  RegisterTarget<Triple::cpu0,
                 /*HasJIT=*/true>
      X(getTheCpu0Target(), "cpu0", "CPU0 (32-bit big endian)", "Cpu0");
}
