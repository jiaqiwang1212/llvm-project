//===-- Cpu0SEInstrInfo.cpp - Cpu032 Instruction Information --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0SEInstrInfo.h"
#include "Cpu0SERegisterInfo.h"

using namespace llvm;

Cpu0SEInstrInfo::Cpu0SEInstrInfo(const Cpu0Subtarget &STI,
                                 const Cpu0SERegisterInfo &RI)
    : Cpu0InstrInfo(STI, RI), RI(RI) {}

const Cpu0RegisterInfo &Cpu0SEInstrInfo::getRegisterInfo() const { return RI; }

const Cpu0InstrInfo *llvm::createCpu0SEInstrInfo(const Cpu0Subtarget &STI,
                                                 const Cpu0RegisterInfo &RI) {
  return new Cpu0SEInstrInfo(STI,
                             static_cast<const Cpu0SERegisterInfo &>(RI));
}
