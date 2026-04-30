//===-- Cpu0MCCodeEmitter.cpp - Convert Cpu0 Code to Machine Code ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Cpu0MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "Cpu0MCCodeEmitter.h"
#include "MCTargetDesc/Cpu0BaseInfo.h"
#include "MCTargetDesc/Cpu0FixupKinds.h"
#include "MCTargetDesc/Cpu0MCExpr.h"
#include "MCTargetDesc/Cpu0MCTargetDesc.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/EndianStream.h"

#define DEBUG_TYPE "mccodeemitter"

#define GET_INSTRMAP_INFO
#include "Cpu0GenInstrInfo.inc"
#undef GET_INSTRMAP_INFO

using namespace llvm;

MCCodeEmitter *llvm::createCpu0MCCodeEmitterEB(const MCInstrInfo &MCII,
                                               MCContext &Ctx) {
  return new Cpu0MCCodeEmitter(MCII, Ctx, false);
}

MCCodeEmitter *llvm::createCpu0MCCodeEmitterEL(const MCInstrInfo &MCII,
                                               MCContext &Ctx) {
  return new Cpu0MCCodeEmitter(MCII, Ctx, true);
}

void Cpu0MCCodeEmitter::encodeInstruction(const MCInst &MI,
                                          SmallVectorImpl<char> &CB,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  uint32_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  uint64_t TSFlags = Desc.TSFlags;

  if ((TSFlags & Cpu0II::FormMask) == Cpu0II::Pseudo)
    llvm_unreachable("Pseudo opcode found in encodeInstruction()");

  support::endian::write<uint32_t>(
      CB, Binary,
      IsLittleEndian ? llvm::endianness::little : llvm::endianness::big);
}

unsigned Cpu0MCCodeEmitter::getExprOpValue(const MCExpr *Expr,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Constant)
    return cast<MCConstantExpr>(Expr)->getValue();

  if (Kind == MCExpr::Binary) {
    unsigned Res =
        getExprOpValue(cast<MCBinaryExpr>(Expr)->getLHS(), Fixups, STI);
    Res += getExprOpValue(cast<MCBinaryExpr>(Expr)->getRHS(), Fixups, STI);
    return Res;
  }

  if (Kind == MCExpr::Target) {
    const Cpu0MCExpr *Cpu0Expr = cast<Cpu0MCExpr>(Expr);
    Cpu0::Fixups FixupKind = Cpu0::Fixups(0);
    switch (Cpu0Expr->getKind()) {
    case Cpu0MCExpr::CEK_ABS_HI:
      FixupKind = Cpu0::fixup_Cpu0_HI16;
      break;
    case Cpu0MCExpr::CEK_ABS_LO:
      FixupKind = Cpu0::fixup_Cpu0_LO16;
      break;
    case Cpu0MCExpr::CEK_GPREL:
      FixupKind = Cpu0::fixup_Cpu0_GPREL16;
      break;
    case Cpu0MCExpr::CEK_GOT:
      FixupKind = Cpu0::fixup_Cpu0_GOT;
      break;
    case Cpu0MCExpr::CEK_GOT_HI16:
      FixupKind = Cpu0::fixup_Cpu0_GOT_HI16;
      break;
    case Cpu0MCExpr::CEK_GOT_LO16:
      FixupKind = Cpu0::fixup_Cpu0_GOT_LO16;
      break;
    default:
      llvm_unreachable("Unsupported fixup kind for target expression!");
    }
    Fixups.push_back(MCFixup::create(0, Expr, MCFixupKind(FixupKind)));
    return 0;
  }

  return 0;
}

unsigned Cpu0MCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                              const MCOperand &MO,
                                              SmallVectorImpl<MCFixup> &Fixups,
                                              const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = Ctx.getRegisterInfo()->getEncodingValue(Reg);
    return RegNo;
  }
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());
  assert(MO.isExpr());
  return getExprOpValue(MO.getExpr(), Fixups, STI);
}

unsigned Cpu0MCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  assert(MI.getOperand(OpNo).isReg());
  unsigned RegBits =
      getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI) << 16;
  unsigned OffBits =
      getMachineOpValue(MI, MI.getOperand(OpNo + 1), Fixups, STI);
  return (OffBits & 0xFFFF) | RegBits;
}

#include "Cpu0GenCodeEmitter.inc"
