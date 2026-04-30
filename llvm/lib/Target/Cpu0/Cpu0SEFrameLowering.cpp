//===-- Cpu0SEFrameLowering.cpp - Cpu0 Frame Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0SEFrameLowering.h"
#include "Cpu0SEInstrInfo.h"
#include "Cpu0Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

Cpu0SEFrameLowering::Cpu0SEFrameLowering(const Cpu0Subtarget &STI)
    : Cpu0FrameLowering(STI, STI.stackAlignment()) {}

void Cpu0SEFrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const Cpu0SEInstrInfo &TII =
      *static_cast<const Cpu0SEInstrInfo *>(MF.getSubtarget().getInstrInfo());

  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  uint64_t StackSize = MFI.getStackSize();

  if (StackSize == 0 && !MFI.adjustsStack())
    return;

  TII.adjustStackPtr(Cpu0::SP, -static_cast<int64_t>(StackSize), MBB, MBBI);
}

void Cpu0SEFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const Cpu0SEInstrInfo &TII =
      *static_cast<const Cpu0SEInstrInfo *>(MF.getSubtarget().getInstrInfo());

  DebugLoc DL = MBBI->getDebugLoc();
  uint64_t StackSize = MFI.getStackSize();

  if (StackSize == 0)
    return;

  TII.adjustStackPtr(Cpu0::SP, static_cast<int64_t>(StackSize), MBB, MBBI);
}

const Cpu0FrameLowering *
llvm::createCpu0SEFrameLowering(const Cpu0Subtarget &ST) {
  return new Cpu0SEFrameLowering(ST);
}
