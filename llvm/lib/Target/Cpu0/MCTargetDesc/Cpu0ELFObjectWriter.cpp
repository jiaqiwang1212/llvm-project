//===-- Cpu0ELFObjectWriter.cpp - Cpu0 ELF Writer -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Cpu0BaseInfo.h"
#include "MCTargetDesc/Cpu0FixupKinds.h"
#include "MCTargetDesc/Cpu0MCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class Cpu0ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  Cpu0ELFObjectWriter(uint8_t OSABI, bool HasRelocationAddend, bool Is64);
  ~Cpu0ELFObjectWriter() override = default;

  unsigned getRelocType(const MCFixup &Fixup, const MCValue &Target,
                        bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCValue &Val,
                               unsigned Type) const override;
};
} // namespace

Cpu0ELFObjectWriter::Cpu0ELFObjectWriter(uint8_t OSABI,
                                         bool HasRelocationAddend, bool Is64)
    : MCELFObjectTargetWriter(Is64, OSABI, ELF::EM_CPU0,
                              HasRelocationAddend) {}

unsigned Cpu0ELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                           const MCValue &Target,
                                           bool IsPCRel) const {
  unsigned Type = (unsigned)ELF::R_CPU0_NONE;
  unsigned Kind = (unsigned)Fixup.getKind();

  switch (Kind) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case FK_Data_4:
    Type = ELF::R_CPU0_32;
    break;
  case Cpu0::fixup_Cpu0_32:
    Type = ELF::R_CPU0_32;
    break;
  case Cpu0::fixup_Cpu0_GPREL16:
    Type = ELF::R_CPU0_GPREL16;
    break;
  case Cpu0::fixup_Cpu0_GOT:
    Type = ELF::R_CPU0_GOT16;
    break;
  case Cpu0::fixup_Cpu0_HI16:
    Type = ELF::R_CPU0_HI16;
    break;
  case Cpu0::fixup_Cpu0_LO16:
    Type = ELF::R_CPU0_LO16;
    break;
  case Cpu0::fixup_Cpu0_GOT_HI16:
    Type = ELF::R_CPU0_GOT_HI16;
    break;
  case Cpu0::fixup_Cpu0_GOT_LO16:
    Type = ELF::R_CPU0_GOT_LO16;
    break;
  }

  return Type;
}

bool Cpu0ELFObjectWriter::needsRelocateWithSymbol(const MCValue &Val,
                                                  unsigned Type) const {
  switch (Type) {
  default:
    return true;
  case ELF::R_CPU0_GOT16:
    return true;
  case ELF::R_CPU0_HI16:
  case ELF::R_CPU0_LO16:
  case ELF::R_CPU0_32:
    return true;
  case ELF::R_CPU0_GPREL16:
    return false;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createCpu0ELFObjectWriter(const Triple &TT) {
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  bool IsN64 = false;
  bool HasRelocationAddend = TT.isArch64Bit();
  return std::make_unique<Cpu0ELFObjectWriter>(OSABI, HasRelocationAddend,
                                               IsN64);
}
