#define DEBUG_TYPE "cpu0-reg-info"

#include "Cpu0RegisterInfo.h"

#include "Cpu0Subtarget.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define GET_REGINFO_ENUM
#include "Cpu0GenRegisterInfo.inc"

#define GET_REGINFO_TARGET_DESC
#include "Cpu0GenRegisterInfo.inc"

Cpu0RegisterInfo::Cpu0RegisterInfo(const Cpu0Subtarget &ST)
    : Cpu0GenRegisterInfo(Cpu0::LR), Subtarget(ST) {}
