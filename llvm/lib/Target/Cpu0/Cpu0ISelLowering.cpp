//===-- Cpu0ISelLowering.cpp - Cpu0 DAG Lowering Implementation -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0ISelLowering.h"
#include "Cpu0Subtarget.h"
#include "Cpu0TargetMachine.h"
#include "Cpu0TargetObjectFile.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/CallingConv.h"

#define DEBUG_TYPE "cpu0-lower"

using namespace llvm;

const char *Cpu0TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case Cpu0ISD::JmpLink:    return "Cpu0ISD::JmpLink";
  case Cpu0ISD::TailCall:   return "Cpu0ISD::TailCall";
  case Cpu0ISD::Hi:         return "Cpu0ISD::Hi";
  case Cpu0ISD::Lo:         return "Cpu0ISD::Lo";
  case Cpu0ISD::GPRel:      return "Cpu0ISD::GPRel";
  case Cpu0ISD::Ret:        return "Cpu0ISD::Ret";
  case Cpu0ISD::EH_RETURN:  return "Cpu0ISD::EH_RETURN";
  case Cpu0ISD::DivRem:     return "Cpu0ISD::DivRem";
  case Cpu0ISD::DivRemU:    return "Cpu0ISD::DivRemU";
  case Cpu0ISD::Wrapper:    return "Cpu0ISD::Wrapper";
  default:                  return nullptr;
  }
}

Cpu0TargetLowering::Cpu0TargetLowering(const Cpu0TargetMachine &TM,
                                       const Cpu0Subtarget &STI)
    : TargetLowering(TM, STI), Subtarget(STI), ABI(TM.getABI()) {}

const Cpu0TargetLowering *
Cpu0TargetLowering::create(const Cpu0TargetMachine &TM,
                           const Cpu0Subtarget &STI) {
  return llvm::createCpu0SETargetLowering(TM, STI);
}

#include "Cpu0GenCallingConv.inc"

SDValue Cpu0TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_Cpu0);

  for (const CCValAssign &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();
      Register VReg = RegInfo.createVirtualRegister(&Cpu0::CPURegsRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);
      if (VA.getLocInfo() == CCValAssign::SExt)
        ArgValue = DAG.getNode(ISD::AssertSext, DL, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
      else if (VA.getLocInfo() == CCValAssign::ZExt)
        ArgValue = DAG.getNode(ISD::AssertZext, DL, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
      InVals.push_back(ArgValue);
    } else {
      assert(VA.isMemLoc());
      unsigned ObjSize = VA.getLocVT().getSizeInBits() / 8;
      int FI = MFI.CreateFixedObject(ObjSize, VA.getLocMemOffset(), true);
      SDValue FIPtr =
          DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue Load =
          DAG.getLoad(VA.getLocVT(), DL, Chain, FIPtr,
                      MachinePointerInfo::getFixedStack(MF, FI));
      InVals.push_back(Load);
    }
  }
  return Chain;
}

SDValue Cpu0TargetLowering::LowerReturn(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
    SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_Cpu0);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned I = 0; I < RVLocs.size(); ++I) {
    const CCValAssign &VA = RVLocs[I];
    assert(VA.isRegLoc() && "Can only return in registers!");
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[I], Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;
  RetOps.push_back(DAG.getRegister(Cpu0::LR, MVT::i32));
  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(Cpu0ISD::Ret, DL, MVT::Other, RetOps);
}
