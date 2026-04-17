//===-- Cpu0Subtarget.cpp - Cpu0 Subtarget Information --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "cpu0-subtarget"

#include "Cpu0Subtarget.h"
#include "Cpu0MachineFunction.h"
#include "Cpu0RegisterInfo.h"
#include "Cpu0TargetMachine.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "Cpu0GenSubtargetInfo.inc"

using namespace llvm;

void Cpu0Subtarget::anchor() {}

Cpu0Subtarget::Cpu0Subtarget(const Triple &TT, StringRef CPU, StringRef FS,
                             bool little, const Cpu0TargetMachine &TM)
    : Cpu0GenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS),
      IsLittle(little), TM(TM), TargetTriple(TT), TSInfo(),
      RegInfo(*this),
      InstrInfo(createCpu0SEInstrInfo(
          initializeSubtargetDependencies(CPU, FS, TM), RegInfo)),
      FrameLowering(Cpu0FrameLowering::create(*this)),
      TLInfo(Cpu0TargetLowering::create(TM, *this)) {}

bool Cpu0Subtarget::isPositionIndependent() const {
  return TM.isPositionIndependent();
}

Cpu0Subtarget &Cpu0Subtarget::initializeSubtargetDependencies(
    StringRef CPU, StringRef FS, const TargetMachine &TM) {
  if (TargetTriple.getArch() == Triple::cpu0 ||
      TargetTriple.getArch() == Triple::cpu0el) {
    if (CPU.empty() || CPU == "generic")
      CPU = "cpu032II";
    else if (CPU != "cpu032I" && CPU != "cpu032II")
      CPU = "cpu032II";
  } else {
    errs() << "Unexpected arch: " << TargetTriple.getArch() << "\n";
    exit(1);
  }

  if (CPU == "cpu032I")
    Cpu0ArchVersion = Cpu032I;
  else
    Cpu0ArchVersion = Cpu032II;

  if (isCpu032I()) {
    HasCmp = true;
    HasSlt = false;
  } else {
    HasCmp = false;
    HasSlt = true;
  }

  ParseSubtargetFeatures(CPU, /*TuneCPU=*/CPU, FS);
  InstrItins = getInstrItineraryForCPU(CPU);
  return *this;
}

const Cpu0ABIInfo &Cpu0Subtarget::getABI() const { return TM.getABI(); }
