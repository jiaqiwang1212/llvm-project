//===-- Cpu0InstPrinter.cpp - Convert Cpu0 MCInst to assembly syntax ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0InstPrinter.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define PRINT_ALIAS_INSTR
#include "Cpu0GenAsmWriter.inc"

static void printExpr(const MCExpr *Expr, const MCAsmInfo *MAI,
                      raw_ostream &OS) {
  if (const auto *CE = dyn_cast<MCConstantExpr>(Expr)) {
    OS << CE->getValue();
    return;
  }
  if (const auto *SRE = dyn_cast<MCSymbolRefExpr>(Expr)) {
    SRE->getSymbol().print(OS, MAI);
    return;
  }
  if (const auto *BE = dyn_cast<MCBinaryExpr>(Expr)) {
    printExpr(BE->getLHS(), MAI, OS);
    switch (BE->getOpcode()) {
    case MCBinaryExpr::Add: OS << '+'; break;
    case MCBinaryExpr::Sub: OS << '-'; break;
    default: OS << '?'; break;
    }
    printExpr(BE->getRHS(), MAI, OS);
    return;
  }
  OS << "<unknown MCExpr>";
}

void Cpu0InstPrinter::printRegName(raw_ostream &OS, MCRegister RegNo) {
  OS << '$' << StringRef(getRegisterName(RegNo)).lower();
}

void Cpu0InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &O) {
  if (!printAliasInstr(MI, Address, O))
    printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void Cpu0InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
    O << Op.getImm();
    return;
  }

  assert(Op.isExpr() && "unknown operand kind in printOperand");
  printExpr(Op.getExpr(), &MAI, O);
}

void Cpu0InstPrinter::printUnsignedImm(const MCInst *MI, int opNum,
                                       raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm())
    O << (unsigned short int)MO.getImm();
  else
    printOperand(MI, opNum, O);
}

void Cpu0InstPrinter::printMemOperand(const MCInst *MI, int opNum,
                                      raw_ostream &O) {
  // Load/Store memory operands -- imm($reg)
  printOperand(MI, opNum + 1, O);
  O << "(";
  printOperand(MI, opNum, O);
  O << ")";
}

void Cpu0InstPrinter::printMemOperandEA(const MCInst *MI, int opNum,
                                        raw_ostream &O) {
  printOperand(MI, opNum, O);
  O << ", ";
  printOperand(MI, opNum + 1, O);
}
