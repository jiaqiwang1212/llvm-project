#include "Cpu0InstrInfo.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "Cpu0GenInstrInfo.inc"

Cpu0InstrInfo::Cpu0InstrInfo(const Cpu0Subtarget &STI,
                             const Cpu0RegisterInfo &RI)
    : Cpu0GenInstrInfo(STI, RI), Subtarget(STI) {}