//===-- Cpu0AnalyzeImmediate.cpp - Analyze Immediates ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0AnalyzeImmediate.h"
#include "Cpu0.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

Cpu0AnalyzeImmediate::Inst::Inst(unsigned O, unsigned I) : Opc(O), ImmOpnd(I) {}

void Cpu0AnalyzeImmediate::AddInstr(InstSeqLs &SeqLs, const Inst &I) {
  if (SeqLs.empty()) {
    SeqLs.push_back(InstSeq(1, I));
    return;
  }
  for (InstSeqLs::iterator Iter = SeqLs.begin(); Iter != SeqLs.end(); ++Iter)
    Iter->push_back(I);
}

void Cpu0AnalyzeImmediate::GetInstSeqLsADDiu(uint64_t Imm, unsigned RemSize,
                                              InstSeqLs &SeqLs) {
  GetInstSeqLs((Imm + 0x8000ULL) & 0xffffffffffff0000ULL, RemSize, SeqLs);
  AddInstr(SeqLs, Inst(ADDiu, Imm & 0xffffULL));
}

void Cpu0AnalyzeImmediate::GetInstSeqLsORi(uint64_t Imm, unsigned RemSize,
                                            InstSeqLs &SeqLs) {
  GetInstSeqLs(Imm & 0xffffffffffff0000ULL, RemSize, SeqLs);
  AddInstr(SeqLs, Inst(ORi, Imm & 0xffffULL));
}

void Cpu0AnalyzeImmediate::GetInstSeqLsSHL(uint64_t Imm, unsigned RemSize,
                                            InstSeqLs &SeqLs) {
  unsigned Shamt = llvm::countr_zero(Imm);
  GetInstSeqLs(Imm >> Shamt, RemSize - Shamt, SeqLs);
  AddInstr(SeqLs, Inst(SHL, Shamt));
}

void Cpu0AnalyzeImmediate::GetInstSeqLs(uint64_t Imm, unsigned RemSize,
                                        InstSeqLs &SeqLs) {
  uint64_t MaskedImm = Imm & (0xffffffffffffffffULL >> (64 - Size));

  if (!MaskedImm)
    return;

  if (RemSize <= 16) {
    AddInstr(SeqLs, Inst(ADDiu, MaskedImm));
    return;
  }

  if (!(Imm & 0xffff)) {
    GetInstSeqLsSHL(Imm, RemSize, SeqLs);
    return;
  }

  GetInstSeqLsADDiu(Imm, RemSize, SeqLs);

  if (Imm & 0x8000) {
    InstSeqLs SeqLsORi;
    GetInstSeqLsORi(Imm, RemSize, SeqLsORi);
    SeqLs.insert(SeqLs.end(), SeqLsORi.begin(), SeqLsORi.end());
  }
}

void Cpu0AnalyzeImmediate::ReplaceADDiuSHLWithLUi(InstSeq &Seq) {
  if ((Seq.size() < 2) || (Seq[0].Opc != ADDiu) ||
      (Seq[1].Opc != SHL) || (Seq[1].ImmOpnd < 16))
    return;

  int64_t Imm = SignExtend64<16>(Seq[0].ImmOpnd);
  int64_t ShiftedImm = (uint64_t)Imm << (Seq[1].ImmOpnd - 16);

  if (!isInt<16>(ShiftedImm))
    return;

  Seq[0].Opc = LUi;
  Seq[0].ImmOpnd = (unsigned)(ShiftedImm & 0xffff);
  Seq.erase(Seq.begin() + 1);
}

void Cpu0AnalyzeImmediate::GetShortestSeq(InstSeqLs &SeqLs, InstSeq &Insts) {
  InstSeqLs::iterator ShortestSeq = SeqLs.end();
  unsigned ShortestLength = 8;

  for (InstSeqLs::iterator S = SeqLs.begin(); S != SeqLs.end(); ++S) {
    ReplaceADDiuSHLWithLUi(*S);
    assert(S->size() <= 7);
    if (S->size() < ShortestLength) {
      ShortestSeq = S;
      ShortestLength = S->size();
    }
  }

  Insts.clear();
  Insts.append(ShortestSeq->begin(), ShortestSeq->end());
}

const Cpu0AnalyzeImmediate::InstSeq &
Cpu0AnalyzeImmediate::Analyze(uint64_t Imm, unsigned Size,
                               bool LastInstrIsADDiu) {
  this->Size = Size;

  ADDiu = Cpu0::ADDiu;
  ORi   = Cpu0::ORi;
  SHL   = Cpu0::SHL;
  LUi   = Cpu0::LUi;

  InstSeqLs SeqLs;

  if (LastInstrIsADDiu | !Imm)
    GetInstSeqLsADDiu(Imm, Size, SeqLs);
  else
    GetInstSeqLs(Imm, Size, SeqLs);

  GetShortestSeq(SeqLs, Insts);
  return Insts;
}
