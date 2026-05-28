#include "Cpu0Subtarget.h"

using namespace llvm;

#define DEBUG_TYPE "cpu0-subtarget"

#define GET_SUBTARGETINFO_ENUM
#include "Cpu0GenSubtargetInfo.inc"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "Cpu0GenSubtargetInfo.inc"

Cpu0Subtarget::Cpu0Subtarget(const Triple &TT, StringRef CPU, StringRef FS)
    : Cpu0GenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS) {
  ParseSubtargetFeatures(CPU, /*TuneCPU=*/CPU, FS);
}
