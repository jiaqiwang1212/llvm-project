//===-- Cpu0AsmBackend.cpp - Cpu0 Asm Backend  ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Cpu0AsmBackend class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Cpu0AsmBackend.h"
#include "MCTargetDesc/Cpu0FixupKinds.h"
#include "MCTargetDesc/Cpu0MCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value) {
  unsigned Kind = Fixup.getKind();
  switch (Kind) {
  default:
    return 0;
  case FK_Data_4:
  case Cpu0::fixup_Cpu0_LO16:
    break;
  case Cpu0::fixup_Cpu0_HI16:
  case Cpu0::fixup_Cpu0_GOT:
    Value = (Value >> 16) & 0xffff;
    break;
  }
  return Value;
}

std::unique_ptr<MCObjectTargetWriter>
Cpu0AsmBackend::createObjectTargetWriter() const {
  return createCpu0ELFObjectWriter(TheTriple);
}

void Cpu0AsmBackend::applyFixup(const MCFragment &, const MCFixup &Fixup,
                                const MCValue &Target, uint8_t *Data,
                                uint64_t Value, bool IsResolved) {
  MCFixupKind Kind = Fixup.getKind();
  Value = adjustFixupValue(Fixup, Value);

  if (!Value)
    return;

  unsigned Offset = Fixup.getOffset();
  unsigned NumBytes = (getFixupKindInfo(Kind).TargetSize + 7) / 8;
  unsigned FullSize = 4;

  uint64_t CurVal = 0;
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned Idx = TheTriple.isLittleEndian() ? i : (FullSize - 1 - i);
    CurVal |= (uint64_t)Data[Offset + Idx] << (i * 8);
  }

  uint64_t Mask = ((uint64_t)(-1) >> (64 - getFixupKindInfo(Kind).TargetSize));
  CurVal |= Value & Mask;

  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned Idx = TheTriple.isLittleEndian() ? i : (FullSize - 1 - i);
    Data[Offset + Idx] = (uint8_t)((CurVal >> (i * 8)) & 0xff);
  }
}

MCFixupKindInfo Cpu0AsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[Cpu0::NumTargetFixupKinds] = {
    // name                     offset  bits  flags
    { "fixup_Cpu0_32",          0,      32,   0 },
    { "fixup_Cpu0_HI16",        0,      16,   0 },
    { "fixup_Cpu0_LO16",        0,      16,   0 },
    { "fixup_Cpu0_GPREL16",     0,      16,   0 },
    { "fixup_Cpu0_GOT",         0,      16,   0 },
    { "fixup_Cpu0_GOT_HI16",    0,      16,   0 },
    { "fixup_Cpu0_GOT_LO16",    0,      16,   0 }
  };

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < Cpu0::NumTargetFixupKinds &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

bool Cpu0AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                  const MCSubtargetInfo *STI) const {
  return true;
}

MCAsmBackend *llvm::createCpu0AsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  return new Cpu0AsmBackend(T, STI.getTargetTriple());
}
