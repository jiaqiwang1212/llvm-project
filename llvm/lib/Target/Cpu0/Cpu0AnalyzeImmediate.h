//===-- Cpu0AnalyzeImmediate.h - Analyze Immediates -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0ANALYZEIMMEDIATE_H
#define LLVM_LIB_TARGET_CPU0_CPU0ANALYZEIMMEDIATE_H

#include "llvm/ADT/SmallVector.h"

namespace llvm {

class Cpu0AnalyzeImmediate {
public:
  struct Inst {
    unsigned Opc, ImmOpnd;
    Inst(unsigned Opc, unsigned ImmOpnd);
  };
  typedef SmallVector<Inst, 7> InstSeq;

  /// Analyze - Get an instruction sequence to load immediate Imm. The last
  /// instruction in the sequence must be an ADDiu if LastInstrIsADDiu is true.
  const InstSeq &Analyze(uint64_t Imm, unsigned Size, bool LastInstrIsADDiu);

private:
  typedef SmallVector<InstSeq, 5> InstSeqLs;

  void AddInstr(InstSeqLs &SeqLs, const Inst &I);
  void GetInstSeqLsADDiu(uint64_t Imm, unsigned RemSize, InstSeqLs &SeqLs);
  void GetInstSeqLsORi(uint64_t Imm, unsigned RemSize, InstSeqLs &SeqLs);
  void GetInstSeqLsSHL(uint64_t Imm, unsigned RemSize, InstSeqLs &SeqLs);
  void GetInstSeqLs(uint64_t Imm, unsigned RemSize, InstSeqLs &SeqLs);
  void ReplaceADDiuSHLWithLUi(InstSeq &Seq);
  void GetShortestSeq(InstSeqLs &SeqLs, InstSeq &Insts);

  unsigned Size;
  unsigned ADDiu, ORi, SHL, LUi;
  InstSeq Insts;
};

} // namespace llvm

#endif
