//===-- Cpu0Subtarget.h - Define Subtarget for the Cpu0 ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0SUBTARGET_H
#define LLVM_LIB_TARGET_CPU0_CPU0SUBTARGET_H

#include "Cpu0FrameLowering.h"
#include "Cpu0ISelLowering.h"
#include "Cpu0InstrInfo.h"
#include "Cpu0SERegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrItineraries.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "Cpu0GenSubtargetInfo.inc"

namespace llvm {

class StringRef;
class Cpu0TargetMachine;

class Cpu0Subtarget : public Cpu0GenSubtargetInfo {
  virtual void anchor();

protected:
  enum Cpu0ArchEnum { Cpu032I, Cpu032II };

  Cpu0ArchEnum Cpu0ArchVersion;
  bool IsLittle;
  bool EnableOverflow;
  bool HasCmp;
  bool HasSlt;
  // Required by generated Cpu0GenSubtargetInfo.inc
  bool HasChapterDummy = false;
  bool HasChapterAll = false;
  InstrItineraryData InstrItins;

  const Cpu0TargetMachine &TM;
  Triple TargetTriple;
  const SelectionDAGTargetInfo TSInfo;

  // RegInfo MUST be declared before InstrInfo (member init order).
  Cpu0SERegisterInfo RegInfo;
  std::unique_ptr<const Cpu0InstrInfo> InstrInfo;
  std::unique_ptr<const Cpu0FrameLowering> FrameLowering;
  std::unique_ptr<const Cpu0TargetLowering> TLInfo;

public:
  Cpu0Subtarget(const Triple &TT, StringRef CPU, StringRef FS, bool little,
                const Cpu0TargetMachine &TM);

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  Cpu0Subtarget &initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                                  const TargetMachine &TM);

  bool isPositionIndependent() const;
  const Cpu0ABIInfo &getABI() const;

  bool isLittle() const { return IsLittle; }
  bool hasCpu032I() const { return Cpu0ArchVersion >= Cpu032I; }
  bool isCpu032I() const { return Cpu0ArchVersion == Cpu032I; }
  bool hasCpu032II() const { return Cpu0ArchVersion >= Cpu032II; }
  bool isCpu032II() const { return Cpu0ArchVersion == Cpu032II; }

  bool enableOverflow() const { return EnableOverflow; }
  bool disableOverflow() const { return !EnableOverflow; }
  bool hasCmp() const { return HasCmp; }
  bool hasSlt() const { return HasSlt; }
  bool abiUsesSoftFloat() const { return true; }
  bool enableLongBranchPass() const { return hasCpu032II(); }
  unsigned stackAlignment() const { return 8; }

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
  const Cpu0InstrInfo *getInstrInfo() const override { return InstrInfo.get(); }
  const TargetFrameLowering *getFrameLowering() const override {
    return FrameLowering.get();
  }
  const Cpu0RegisterInfo *getRegisterInfo() const override { return &RegInfo; }
  const Cpu0TargetLowering *getTargetLowering() const override {
    return TLInfo.get();
  }
  const InstrItineraryData *getInstrItineraryData() const override {
    return &InstrItins;
  }
};

} // namespace llvm

#endif
