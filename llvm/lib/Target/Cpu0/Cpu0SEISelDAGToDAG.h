//===-- Cpu0SEISelDAGToDAG.h - A Dag to Dag Inst Selector for Cpu0SE ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Subclass of Cpu0DAGToDAGISel specialized for cpu032.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0SEISELDAGTODAG_H
#define LLVM_LIB_TARGET_CPU0_CPU0SEISELDAGTODAG_H

#include "Cpu0ISelDAGToDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"

namespace llvm {

class Cpu0SEDAGToDAGISel : public Cpu0DAGToDAGISel {
public:
  explicit Cpu0SEDAGToDAGISel(Cpu0TargetMachine &TM, CodeGenOptLevel OL)
      : Cpu0DAGToDAGISel(TM, OL) {}

private:
  bool runOnMachineFunction(MachineFunction &MF) override;
  bool trySelect(SDNode *Node) override;
  void processFunctionAfterISel(MachineFunction &MF) override;

  std::pair<SDNode *, SDNode *> selectMULT(SDNode *N, unsigned Opc,
                                           const SDLoc &DL, EVT Ty,
                                           bool HasLo, bool HasHi);
};

class Cpu0SEDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  explicit Cpu0SEDAGToDAGISelLegacy(Cpu0TargetMachine &TM, CodeGenOptLevel OL);
};

FunctionPass *createCpu0SEISelDag(Cpu0TargetMachine &TM, CodeGenOptLevel OL);

} // namespace llvm

#endif
