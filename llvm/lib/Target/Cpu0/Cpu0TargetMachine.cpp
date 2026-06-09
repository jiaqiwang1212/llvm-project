
#include "Cpu0TargetMachine.h"
#include "TargetInfo/Cpu0TargetInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "cpu0"

static Reloc::Model getEffectiveRelocModel(bool JIT,
                                           std::optional<Reloc::Model> RM) {
  if (!RM.has_value() || JIT)
    return Reloc::Static;
  return *RM;
}
Cpu0TargetMachine::Cpu0TargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(
          T, TT.computeDataLayout(Options.MCOptions.getABIName()), TT, CPU, FS,
          Options, getEffectiveRelocModel(JIT, RM),
          getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<Cpu0TargetObjectFile>()),
      DefaultSubtarget(TT, CPU, FS) {
  initAsmInfo();
}
// big endian target machine
Cpu0ebTargetMachine::Cpu0ebTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : Cpu0TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT) {}
// little endian target machine
Cpu0elTargetMachine::Cpu0elTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : Cpu0TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT) {}

TargetPassConfig *Cpu0TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TargetPassConfig(*this, PM);
}

// code生成的入口，注册pass，包括指令选择，寄存器分配，sheduler等
// 这个会注册到llvm的全局TargetMachine中，llvm在生成代码时会根据目标平台选择对应的TargetMachine
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0Target() {
  // Register the target.
  RegisterTargetMachine<Cpu0ebTargetMachine> X(getTheCpu0Target());
  RegisterTargetMachine<Cpu0elTargetMachine> Y(getTheCpu0elTarget());
}
