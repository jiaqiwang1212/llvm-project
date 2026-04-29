//===-- Cpu0SEISelLowering.cpp - Cpu0SE DAG Lowering Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0SEISelLowering.h"
#include "Cpu0RegisterInfo.h"
#include "Cpu0TargetMachine.h"
#include "llvm/CodeGen/SelectionDAG.h"

#define DEBUG_TYPE "cpu0-isel"

using namespace llvm;

static SDValue performDivRemCombine(SDNode *N, SelectionDAG &DAG,
                                     TargetLowering::DAGCombinerInfo &DCI,
                                     const Cpu0Subtarget &Subtarget) {
  if (DCI.isBeforeLegalizeOps())
    return SDValue();

  EVT Ty = N->getValueType(0);
  unsigned LO = Cpu0::LO;
  unsigned HI = Cpu0::HI;
  unsigned Opc = N->getOpcode() == ISD::SDIVREM ? Cpu0ISD::DivRem
                                                 : Cpu0ISD::DivRemU;
  SDLoc DL(N);

  // Emit glue-only DivRem node; ISel matches it to SDIV/UDIV via TableGen pattern.
  SDValue DivRem = DAG.getNode(Opc, DL, MVT::Glue,
                                N->getOperand(0), N->getOperand(1));
  SDValue InChain = DAG.getEntryNode();
  SDValue InGlue = DivRem;

  // Quotient in LO.
  if (N->hasAnyUseOfValue(0)) {
    SDValue CopyFromLo = DAG.getCopyFromReg(InChain, DL, LO, Ty, InGlue);
    DAG.ReplaceAllUsesOfValueWith(SDValue(N, 0), CopyFromLo);
    InChain = CopyFromLo.getValue(1);
    InGlue  = CopyFromLo.getValue(2);
  }

  // Remainder in HI.
  if (N->hasAnyUseOfValue(1)) {
    SDValue CopyFromHi = DAG.getCopyFromReg(InChain, DL, HI, Ty, InGlue);
    DAG.ReplaceAllUsesOfValueWith(SDValue(N, 1), CopyFromHi);
  }

  return SDValue();
}

Cpu0SETargetLowering::Cpu0SETargetLowering(const Cpu0TargetMachine &TM,
                                           const Cpu0Subtarget &STI)
    : Cpu0TargetLowering(TM, STI) {
  addRegisterClass(MVT::i32, &Cpu0::CPURegsRegClass);

  setOperationAction(ISD::MULHS, MVT::i32, Legal);
  setOperationAction(ISD::MULHU, MVT::i32, Legal);
  setOperationAction(ISD::SDIV,  MVT::i32, Expand);
  setOperationAction(ISD::SREM,  MVT::i32, Expand);
  setOperationAction(ISD::UDIV,  MVT::i32, Expand);
  setOperationAction(ISD::UREM,  MVT::i32, Expand);
  setTargetDAGCombine({ISD::SDIVREM, ISD::UDIVREM});

  computeRegisterProperties(Subtarget.getRegisterInfo());
}

SDValue Cpu0SETargetLowering::PerformDAGCombine(SDNode *N,
                                                 DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  switch (N->getOpcode()) {
  default:
    break;
  case ISD::SDIVREM:
  case ISD::UDIVREM:
    return performDivRemCombine(N, DAG, DCI, *static_cast<const Cpu0Subtarget *>(&Subtarget));
  }
  return SDValue();
}

const Cpu0TargetLowering *
llvm::createCpu0SETargetLowering(const Cpu0TargetMachine &TM,
                                 const Cpu0Subtarget &STI) {
  return new Cpu0SETargetLowering(TM, STI);
}
