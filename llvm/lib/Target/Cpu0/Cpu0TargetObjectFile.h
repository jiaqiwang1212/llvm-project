#ifndef LLVM_LIB_TARGET_MIPS_MIPSTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_MIPS_MIPSTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {
class Cpu0TargetMachine;
class Cpu0TargetObjectFile : public TargetLoweringObjectFileELF {
  MCSection *SmallDataSection;
  MCSection *SmallBSSSection;
  const Cpu0TargetMachine *TM;

public:
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;
};
} // end namespace llvm

#endif // LLVM_LIB_TARGET_MIPS_MIPSTARGETOBJECTFILE_H