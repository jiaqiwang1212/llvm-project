#ifndef LLVM_LIB_TARGET_CPU0_CPU0SUBTARGET_H
#define LLVM_LIB_TARGET_CPU0_CPU0SUBTARGET_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCInstrItineraries.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "Cpu0GenSubtargetInfo.inc"

namespace llvm {
class Cpu0Subtarget : public Cpu0GenSubtargetInfo {
  enum Cpu0ArchEnum { Cpu032I, Cpu032II };

  bool HasCmp = false;
  bool HasSlt = false;
  bool IsO32 = false;
  bool IsS32 = false;
  Cpu0ArchEnum Cpu0ArchVersion = Cpu032I;

public:
  bool HasChapterDummy = false;
  bool HasChapterAll = false;

  Cpu0Subtarget(const Triple &TT, StringRef CPU, StringRef FS);

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  const TargetRegisterInfo *getRegisterInfo() const override { return nullptr; }

  bool hasChapter31() const {
#if CH >= CH3_1
    return true;
#else
    return false;
#endif
  }
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_CPU0_CPU0SUBTARGET_H
