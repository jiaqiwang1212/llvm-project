#include "TargetInfo/Cpu0TargetInfo.h"
#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

namespace {
class Cpu0MCAsmInfo : public MCAsmInfoELF {
public:
  explicit Cpu0MCAsmInfo(const Triple &TT) {
    CodePointerSize = 4;
    CalleeSaveStackSlotSize = 4;
    IsLittleEndian = TT.isLittleEndian();
  }
};
} // end anonymous namespace

#define GET_SUBTARGETINFO_ENUM
#include "Cpu0GenSubtargetInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Cpu0GenSubtargetInfo.inc"

static MCSubtargetInfo *createCpu0MCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU,
                                                  StringRef FS) {
  return createCpu0MCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCAsmInfo *createCpu0MCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  return new Cpu0MCAsmInfo(TT);
}

// 初始化 Cpu0 的Machine Code 层，包括指令描述，寄存器信息，汇编语法等，最终会注册MCInstrInfo, MCRegisterInfo,
// MCAsmInfo等
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0TargetMC() {
  for(Target *T : {&getTheCpu0Target(), &getTheCpu0elTarget()}) {
    TargetRegistry::RegisterMCAsmInfo(*T, createCpu0MCAsmInfo);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createCpu0MCSubtargetInfo);
  }
}
