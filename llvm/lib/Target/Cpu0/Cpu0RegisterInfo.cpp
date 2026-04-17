//===-- Cpu0RegisterInfo.cpp - CPU0 Register Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "cpu0-reg-info"

#include "Cpu0RegisterInfo.h"
#include "Cpu0Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "Cpu0GenRegisterInfo.inc"

using namespace llvm;

Cpu0RegisterInfo::Cpu0RegisterInfo(const Cpu0Subtarget &ST)
    : Cpu0GenRegisterInfo(Cpu0::LR), Subtarget(ST) {}

const MCPhysReg *
Cpu0RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_O32_SaveList;
}

const uint32_t *
Cpu0RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const {
  return CSR_O32_RegMask;
}

BitVector Cpu0RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  static const uint16_t ReservedCPURegs[] = {
      Cpu0::ZERO, Cpu0::AT, Cpu0::SP, Cpu0::LR, Cpu0::PC};
  BitVector Reserved(getNumRegs());
  for (unsigned Reg : ReservedCPURegs)
    Reserved.set(Reg);
  if (MF.getSubtarget().getFrameLowering()->hasFP(MF))
    Reserved.set(Cpu0::FP);
  return Reserved;
}

bool Cpu0RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  int64_t SpOffset = MFI.getObjectOffset(FrameIndex);
  uint64_t StackSize = MFI.getStackSize();

  // Determine whether to use SP or FP as the base register.
  // Use FP for non-callee-saved frame objects when FP is active.
  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();
  int MinCSFI = INT_MAX, MaxCSFI = INT_MIN;
  for (const auto &I : CSI) {
    MinCSFI = std::min(MinCSFI, I.getFrameIdx());
    MaxCSFI = std::max(MaxCSFI, I.getFrameIdx());
  }

  Register FrameReg;
  if (TFI->hasFP(MF) &&
      !(FrameIndex >= MinCSFI && FrameIndex <= MaxCSFI))
    FrameReg = Cpu0::FP;
  else
    FrameReg = Cpu0::SP;

  int64_t Offset = (FrameReg == Cpu0::SP)
                       ? SpOffset + static_cast<int64_t>(StackSize)
                       : SpOffset + 4;

  bool HasImmOp = FIOperandNum + 1 < MI.getNumOperands() &&
                  MI.getOperand(FIOperandNum + 1).isImm();
  if (HasImmOp)
    Offset += MI.getOperand(FIOperandNum + 1).getImm();

  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false);
  if (HasImmOp)
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);

  return false;
}

bool Cpu0RegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return true;
}

Register
Cpu0RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  return TFI->hasFP(MF) ? Cpu0::FP : Cpu0::SP;
}
