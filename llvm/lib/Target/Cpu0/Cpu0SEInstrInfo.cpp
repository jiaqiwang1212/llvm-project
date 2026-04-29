//===-- Cpu0SEInstrInfo.cpp - Cpu032 Instruction Information --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0SEInstrInfo.h"
#include "Cpu0SERegisterInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"

using namespace llvm;

Cpu0SEInstrInfo::Cpu0SEInstrInfo(const Cpu0Subtarget &STI,
                                 const Cpu0SERegisterInfo &RI)
    : Cpu0InstrInfo(STI, RI), RI(RI) {}

const Cpu0RegisterInfo &Cpu0SEInstrInfo::getRegisterInfo() const { return RI; }

void Cpu0SEInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator I,
                                   const DebugLoc &DL,
                                   Register DestReg, Register SrcReg,
                                   bool KillSrc, bool RenamableDest,
                                   bool RenamableSrc) const {
  unsigned Opc = 0;

  if (Cpu0::GPROutRegClass.contains(DestReg)) {
    if (Cpu0::GPROutRegClass.contains(SrcReg))
      Opc = Cpu0::ADDu;   // GPR -> GPR: ADDu dst, ZERO, src
    else if (SrcReg == Cpu0::HI)
      Opc = Cpu0::MFHI;
    else if (SrcReg == Cpu0::LO)
      Opc = Cpu0::MFLO;
  } else if (DestReg == Cpu0::HI) {
    if (Cpu0::GPROutRegClass.contains(SrcReg))
      Opc = Cpu0::MTHI;
  } else if (DestReg == Cpu0::LO) {
    if (Cpu0::GPROutRegClass.contains(SrcReg))
      Opc = Cpu0::MTLO;
  }

  assert(Opc && "Cannot copy registers");

  MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Opc));
  if (Opc == Cpu0::ADDu) {
    MIB.addReg(DestReg, RegState::Define)
       .addReg(Cpu0::ZERO)
       .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (Opc == Cpu0::MFHI || Opc == Cpu0::MFLO) {
    MIB.addReg(DestReg, RegState::Define);
  } else {
    // MTHI / MTLO
    MIB.addReg(SrcReg, getKillRegState(KillSrc));
  }
}

void Cpu0SEInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register SrcReg,
    bool IsKill, int FI, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOStore,
      MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  BuildMI(MBB, I, DL, get(Cpu0::ST))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addFrameIndex(FI)
      .addImm(0)
      .addMemOperand(MMO)
      .setMIFlags(Flags);
}

void Cpu0SEInstrInfo::loadRegFromStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register DestReg,
    int FI, const TargetRegisterClass *RC, Register VReg, unsigned SubReg,
    MachineInstr::MIFlag Flags) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOLoad,
      MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  BuildMI(MBB, I, DL, get(Cpu0::LD), DestReg)
      .addFrameIndex(FI)
      .addImm(0)
      .addMemOperand(MMO)
      .setMIFlags(Flags);
}

const Cpu0InstrInfo *llvm::createCpu0SEInstrInfo(const Cpu0Subtarget &STI,
                                                 const Cpu0RegisterInfo &RI) {
  return new Cpu0SEInstrInfo(STI,
                             static_cast<const Cpu0SERegisterInfo &>(RI));
}
