//===-- Cpu0ISelDAGToDAG.h - A Dag to Dag Inst Selector for Cpu0 ---------===//
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

#ifndef LLVM_LIB_TARGET_CPU0_CPU0ISELDAGTODAG_H
#define LLVM_LIB_TARGET_CPU0_CPU0ISELDAGTODAG_H

#include "Cpu0.h"
#include "Cpu0Subtarget.h"
#include "Cpu0TargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"

namespace llvm {

class Cpu0DAGToDAGISel : public SelectionDAGISel {
  const Cpu0Subtarget *Subtarget = nullptr;

public:
  Cpu0DAGToDAGISel() = delete;
  explicit Cpu0DAGToDAGISel(Cpu0TargetMachine &TM, CodeGenOptLevel OL)
      : SelectionDAGISel(TM, OL) {}

  bool runOnMachineFunction(MachineFunction &MF) override;
  void Select(SDNode *Node) override;
  StringRef getPassName() const;

  bool SelectAddr(SDValue N, SDValue &Base, SDValue &Offset);

private:
#include "Cpu0GenDAGISel.inc"
};

class Cpu0DAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  explicit Cpu0DAGToDAGISelLegacy(Cpu0TargetMachine &TM, CodeGenOptLevel OL);
};

} // namespace llvm

#endif
