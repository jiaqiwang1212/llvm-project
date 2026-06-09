#ifndef LLVM_LIB_TARGET_CPU0_CPU0FRAMELOWERING_H
#define LLVM_LIB_TARGET_CPU0_CPU0FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

using namespace llvm;

namespace llvm {
class Cpu0FrameLowering : public TargetFrameLowering {
public:
  Cpu0FrameLowering();
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_CPU0_CPU0FRAMELOWERING_H