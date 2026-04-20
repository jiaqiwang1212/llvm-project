//===-- Cpu0MachineFunction.h - Private data used for Cpu0 ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0MACHINEFUNCTION_H
#define LLVM_LIB_TARGET_CPU0_CPU0MACHINEFUNCTION_H

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Function.h"

namespace llvm {

class Cpu0FunctionInfo : public MachineFunctionInfo {
  int VarArgsFrameIndex = 0;
  bool EmitNOAT = false;

  virtual void anchor();

public:
  explicit Cpu0FunctionInfo(const Function &F, const TargetSubtargetInfo *STI) {}

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  bool getEmitNOAT() const { return EmitNOAT; }
  void setEmitNOAT() { EmitNOAT = true; }
};

} // namespace llvm

#endif
