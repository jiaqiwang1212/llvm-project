#include "TargetInfo/Cpu0TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheCpu0Target() {
  static Target TheCpu0Target;
  return TheCpu0Target;
}

Target &llvm::getTheCpu0elTarget() {
  static Target TheCpu0elTarget;
  return TheCpu0elTarget;
}

// 注册 Cpu0 target 到llvm的全局Target中，为了llvm可以识别 Cpu0
// 这个当使用llc时会显示
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeCpu0TargetInfo() {
  RegisterTarget<Triple::cpu0,
                 /*HasJIT=*/true>
      X(getTheCpu0Target(), "cpu0", "CPU0 (32-bit big endian)", "Cpu0");
  RegisterTarget<Triple::cpu0el,
                 /*HasJIT=*/true>
      Y(getTheCpu0elTarget(), "cpu0el", "CPU0 (32-bit little endian)", "Cpu0");
}
