//===---- Cpu0ABIInfo.h - Information about CPU0 ABI's --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_MCTARGETDESC_CPU0ABIINFO_H
#define LLVM_LIB_TARGET_CPU0_MCTARGETDESC_CPU0ABIINFO_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace llvm {

class Cpu0ABIInfo {
public:
  enum class ABI { Unknown, O32, S32 };

protected:
  ABI ThisABI;

public:
  Cpu0ABIInfo(ABI ThisABI) : ThisABI(ThisABI) {}

  static Cpu0ABIInfo Unknown() { return Cpu0ABIInfo(ABI::Unknown); }
  static Cpu0ABIInfo O32() { return Cpu0ABIInfo(ABI::O32); }
  static Cpu0ABIInfo S32() { return Cpu0ABIInfo(ABI::S32); }
  static Cpu0ABIInfo computeTargetABI();

  bool IsKnown() const { return ThisABI != ABI::Unknown; }
  bool IsO32() const { return ThisABI == ABI::O32; }
  bool IsS32() const { return ThisABI == ABI::S32; }
  ABI GetEnumValue() const { return ThisABI; }

  const ArrayRef<MCPhysReg> GetByValArgRegs() const;
  const ArrayRef<MCPhysReg> GetVarArgRegs() const;
  unsigned GetCalleeAllocdArgSizeInBytes(CallingConv::ID CC) const;

  unsigned GetStackPtr() const;
  unsigned GetFramePtr() const;
  unsigned GetNullPtr() const;
  unsigned GetEhDataReg(unsigned I) const;
  int EhDataRegSize() const;
};

} // namespace llvm

#endif
