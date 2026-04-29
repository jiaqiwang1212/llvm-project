//===-- Cpu0ISelDAGToDAG.cpp - A Dag to Dag Inst Selector for Cpu0 -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the Cpu0 target.
//
//===----------------------------------------------------------------------===//

#include "Cpu0ISelDAGToDAG.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "cpu0-isel"
#define PASS_NAME "Cpu0 DAG->DAG Pattern Instruction Selection"

using namespace llvm;

bool Cpu0DAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &MF.getSubtarget<Cpu0Subtarget>();
  return SelectionDAGISel::runOnMachineFunction(MF);
}

void Cpu0DAGToDAGISel::selectMULT(SDNode *N, unsigned Opc, const SDLoc &DL,
                                    EVT Ty, bool HasHi, bool HasLo) {
  SDNode *Mul = CurDAG->getMachineNode(Opc, DL, MVT::Glue,
                                        N->getOperand(0), N->getOperand(1));
  SDValue InGlue = SDValue(Mul, 0);

  if (HasHi) {
    SDNode *Hi = CurDAG->getMachineNode(Cpu0::MFHI, DL, Ty, MVT::Glue, InGlue);
    InGlue = SDValue(Hi, 1);
    ReplaceNode(N, Hi);
  }
  if (HasLo) {
    SDNode *Lo = CurDAG->getMachineNode(Cpu0::MFLO, DL, Ty, MVT::Glue, InGlue);
    ReplaceNode(N, Lo);
  }
}

void Cpu0DAGToDAGISel::Select(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();
  SDLoc DL(Node);

  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(dbgs() << "== "; Node->dump(CurDAG); dbgs() << "\n");
    Node->setNodeId(-1);
    return;
  }

  switch (Opcode) {
  default:
    break;
  case ISD::MULHS:
    selectMULT(Node, Cpu0::MULT,  DL, MVT::i32, /*HasHi=*/true, /*HasLo=*/false);
    return;
  case ISD::MULHU:
    selectMULT(Node, Cpu0::MULTu, DL, MVT::i32, /*HasHi=*/true, /*HasLo=*/false);
    return;
  }

  SelectCode(Node);
}

StringRef Cpu0DAGToDAGISel::getPassName() const { return PASS_NAME; }

bool Cpu0DAGToDAGISel::SelectAddr(SDValue N, SDValue &Base, SDValue &Offset) {
  EVT ValTy = N.getValueType();
  SDLoc DL(N);

  if (FrameIndexSDNode *FI = dyn_cast<FrameIndexSDNode>(N)) {
    Base = CurDAG->getTargetFrameIndex(FI->getIndex(), ValTy);
    Offset = CurDAG->getTargetConstant(0, DL, ValTy);
    return true;
  }

  Base = N;
  Offset = CurDAG->getTargetConstant(0, DL, ValTy);
  return true;
}

char Cpu0DAGToDAGISelLegacy::ID = 0;

Cpu0DAGToDAGISelLegacy::Cpu0DAGToDAGISelLegacy(Cpu0TargetMachine &TM,
                                               CodeGenOptLevel OL)
    : SelectionDAGISelLegacy(ID, std::make_unique<Cpu0DAGToDAGISel>(TM, OL)) {}

INITIALIZE_PASS(Cpu0DAGToDAGISelLegacy, DEBUG_TYPE, PASS_NAME, false, false)

FunctionPass *llvm::createCpu0ISelDag(Cpu0TargetMachine &TM,
                                      CodeGenOptLevel OL) {
  return new Cpu0DAGToDAGISelLegacy(TM, OL);
}
