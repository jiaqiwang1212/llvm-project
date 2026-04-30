//===-- Cpu0InstrInfo.h - Cpu0 Instruction Information ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0INSTRINFO_H
#define LLVM_LIB_TARGET_CPU0_CPU0INSTRINFO_H

#include "Cpu0RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "Cpu0GenInstrInfo.inc"

namespace llvm {

class Cpu0Subtarget;

class Cpu0InstrInfo : public Cpu0GenInstrInfo {
protected:
  const Cpu0Subtarget &Subtarget;

public:
  explicit Cpu0InstrInfo(const Cpu0Subtarget &STI, const Cpu0RegisterInfo &RI);

  virtual const Cpu0RegisterInfo &getRegisterInfo() const = 0;

  unsigned GetInstSizeInBytes(const MachineInstr &MI) const;

  virtual void adjustStackPtr(unsigned SP, int64_t Amount,
                              MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const = 0;
};

const Cpu0InstrInfo *createCpu0SEInstrInfo(const Cpu0Subtarget &STI,
                                           const Cpu0RegisterInfo &RI);

} // namespace llvm

#endif
