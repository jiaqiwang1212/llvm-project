//===-- Cpu0AsmPrinter.cpp - Cpu0 LLVM Assembly Printer -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0AsmPrinter.h"
#include "InstPrinter/Cpu0InstPrinter.h"
#include "MCTargetDesc/Cpu0BaseInfo.h"
#include "TargetInfo/Cpu0TargetInfo.h"
#include "Cpu0.h"
#include "Cpu0InstrInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "cpu0-asm-printer"

bool Cpu0AsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();
  AsmPrinter::runOnMachineFunction(MF);
  return true;
}

void Cpu0AsmPrinter::emitInstruction(const MachineInstr *MI) {
  if (MI->isDebugValue()) {
    SmallString<128> Str;
    raw_svector_ostream OS(Str);
    PrintDebugValueComment(MI, OS);
    return;
  }

  MachineBasicBlock::const_instr_iterator I = MI->getIterator();
  MachineBasicBlock::const_instr_iterator E = MI->getParent()->instr_end();

  do {
    if (I->isPseudo())
      llvm_unreachable("Pseudo opcode found in emitInstruction()");

    MCInst TmpInst0;
    MCInstLowering.Lower(&*I, TmpInst0);
    OutStreamer->emitInstruction(TmpInst0, getSubtargetInfo());
  } while ((++I != E) && I->isInsideBundle());
}

void Cpu0AsmPrinter::printSavedRegsBitmask(raw_ostream &O) {
  unsigned CPUBitmask = 0;
  int CPUTopSavedRegOff;

  const MachineFrameInfo &MFI = MF->getFrameInfo();
  const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();
  unsigned CPURegSize = TRI->getRegSizeInBits(Cpu0::CPURegsRegClass) / 8;

  for (unsigned i = 0, e = CSI.size(); i != e; ++i) {
    unsigned Reg = CSI[i].getReg();
    unsigned RegNum = TRI->getEncodingValue(Reg);
    CPUBitmask |= (1 << RegNum);
  }

  CPUTopSavedRegOff = CPUBitmask ? -CPURegSize : 0;

  O << "\t.mask \t";
  printHex32(CPUBitmask, O);
  O << ',' << CPUTopSavedRegOff << '\n';
}

void Cpu0AsmPrinter::printHex32(unsigned Value, raw_ostream &O) {
  O << "0x";
  for (int i = 7; i >= 0; i--)
    O.write_hex((Value & (0xF << (i * 4))) >> (i * 4));
}

void Cpu0AsmPrinter::emitFrameDirective() {
  const TargetRegisterInfo &RI = *MF->getSubtarget().getRegisterInfo();

  unsigned stackReg = RI.getFrameRegister(*MF);
  unsigned returnReg = RI.getRARegister();
  unsigned stackSize = MF->getFrameInfo().getStackSize();

  if (OutStreamer->hasRawTextSupport())
    OutStreamer->emitRawText(
        "\t.frame\t$" +
        StringRef(Cpu0InstPrinter::getRegisterName(stackReg)).lower() + "," +
        Twine(stackSize) + ",$" +
        StringRef(Cpu0InstPrinter::getRegisterName(returnReg)).lower());
}

const char *Cpu0AsmPrinter::getCurrentABIString() const {
  switch (static_cast<Cpu0TargetMachine &>(TM).getABI().GetEnumValue()) {
  case Cpu0ABIInfo::ABI::O32:
    return "abiO32";
  case Cpu0ABIInfo::ABI::S32:
    return "abiS32";
  default:
    llvm_unreachable("Unknown Cpu0 ABI");
  }
}

void Cpu0AsmPrinter::emitFunctionEntryLabel() {
  if (OutStreamer->hasRawTextSupport())
    OutStreamer->emitRawText("\t.ent\t" + Twine(CurrentFnSym->getName()));
  OutStreamer->emitLabel(CurrentFnSym);
}

void Cpu0AsmPrinter::emitFunctionBodyStart() {
  MCInstLowering.Initialize(&MF->getContext());

  if (OutStreamer->hasRawTextSupport()) {
    emitFrameDirective();
    SmallString<128> Str;
    raw_svector_ostream OS(Str);
    printSavedRegsBitmask(OS);
    OutStreamer->emitRawText(OS.str());
    OutStreamer->emitRawText(StringRef("\t.set\tnoreorder"));
    OutStreamer->emitRawText(StringRef("\t.set\tnomacro"));
    if (Cpu0FI->getEmitNOAT())
      OutStreamer->emitRawText(StringRef("\t.set\tnoat"));
  }
}

void Cpu0AsmPrinter::emitFunctionBodyEnd() {
  if (OutStreamer->hasRawTextSupport()) {
    if (Cpu0FI->getEmitNOAT())
      OutStreamer->emitRawText(StringRef("\t.set\tat"));
    OutStreamer->emitRawText(StringRef("\t.set\tmacro"));
    OutStreamer->emitRawText(StringRef("\t.set\treorder"));
    OutStreamer->emitRawText("\t.end\t" + Twine(CurrentFnSym->getName()));
  }
}

void Cpu0AsmPrinter::emitStartOfAsmFile(Module &M) {
  if (OutStreamer->hasRawTextSupport())
    OutStreamer->emitRawText("\t.section .mdebug." +
                            Twine(getCurrentABIString()));

  if (OutStreamer->hasRawTextSupport())
    OutStreamer->emitRawText(StringRef("\t.previous"));
}

void Cpu0AsmPrinter::PrintDebugValueComment(const MachineInstr *MI,
                                            raw_ostream &OS) {
  OS << "PrintDebugValueComment()";
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0AsmPrinter() {
  RegisterAsmPrinter<Cpu0AsmPrinter> X(getTheCpu0Target());
  RegisterAsmPrinter<Cpu0AsmPrinter> Y(getTheCpu0elTarget());
}
