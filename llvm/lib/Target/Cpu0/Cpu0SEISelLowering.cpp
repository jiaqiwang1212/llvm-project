//===-- Cpu0SEISelLowering.cpp - Cpu0SE DAG Lowering Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0SEISelLowering.h"
#include "Cpu0TargetMachine.h"

#define DEBUG_TYPE "cpu0-isel"

using namespace llvm;

Cpu0SETargetLowering::Cpu0SETargetLowering(const Cpu0TargetMachine &TM,
                                           const Cpu0Subtarget &STI)
    : Cpu0TargetLowering(TM, STI) {
  addRegisterClass(MVT::i32, &Cpu0::CPURegsRegClass);
  computeRegisterProperties(Subtarget.getRegisterInfo());
}

const Cpu0TargetLowering *
llvm::createCpu0SETargetLowering(const Cpu0TargetMachine &TM,
                                 const Cpu0Subtarget &STI) {
  return new Cpu0SETargetLowering(TM, STI);
}
