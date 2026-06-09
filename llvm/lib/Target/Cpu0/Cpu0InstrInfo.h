#ifndef LLVM_LIB_TARGET_CPU0_CPU0INSTRINFO_H
#define LLVM_LIB_TARGET_CPU0_CPU0INSTRINFO_H

#include "llvm/CodeGen/TargetInstrInfo.h"
#include "Cpu0Subtarget.h"
#include "Cpu0RegisterInfo.h"
#define GET_INSTRINFO_HEADER
#include "Cpu0GenInstrInfo.inc"

namespace llvm {
class Cpu0InstrInfo : public Cpu0GenInstrInfo {
protected:
    const Cpu0Subtarget &Subtarget;
public:
  Cpu0InstrInfo(const Cpu0Subtarget &STI, const Cpu0RegisterInfo &RI);
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_CPU0_CPU0INSTRINFO_H
