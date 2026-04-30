//===-- Cpu0MCTargetDesc.cpp - Cpu0 Target Descriptions --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Cpu0 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "Cpu0MCAsmInfo.h"
#include "Cpu0MCTargetDesc.h"
#include "Cpu0TargetStreamer.h"
#include "InstPrinter/Cpu0InstPrinter.h"
#include "TargetInfo/Cpu0TargetInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "Cpu0GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Cpu0GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "Cpu0GenRegisterInfo.inc"

static MCInstrInfo *createCpu0MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitCpu0MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createCpu0MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitCpu0MCRegisterInfo(X, Cpu0::LR);
  return X;
}

static MCSubtargetInfo *createCpu0MCSubtargetInfo(const Triple &TT,
                                                   StringRef CPU,
                                                   StringRef FS) {
  return createCpu0MCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCAsmInfo *createCpu0MCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  return new Cpu0MCAsmInfo(TT);
}

static MCInstPrinter *createCpu0MCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new Cpu0InstPrinter(MAI, MII, MRI);
}

static MCStreamer *createMCStreamer(const Triple &TT, MCContext &Context,
                                   std::unique_ptr<MCAsmBackend> &&MAB,
                                   std::unique_ptr<MCObjectWriter> &&OW,
                                   std::unique_ptr<MCCodeEmitter> &&Emitter) {
  return createELFStreamer(Context, std::move(MAB), std::move(OW),
                           std::move(Emitter));
}

static MCTargetStreamer *
createCpu0AsmTargetStreamer(MCStreamer &S, formatted_raw_ostream &OS,
                            MCInstPrinter *InstPrint) {
  return new Cpu0TargetAsmStreamer(S, OS);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0TargetMC() {
  for (Target *T : {&getTheCpu0Target(), &getTheCpu0elTarget()}) {
    TargetRegistry::RegisterMCAsmInfo(*T, createCpu0MCAsmInfo);
    TargetRegistry::RegisterMCInstrInfo(*T, createCpu0MCInstrInfo);
    TargetRegistry::RegisterMCRegInfo(*T, createCpu0MCRegisterInfo);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createCpu0MCSubtargetInfo);
    TargetRegistry::RegisterMCInstPrinter(*T, createCpu0MCInstPrinter);
    TargetRegistry::RegisterELFStreamer(*T, createMCStreamer);
    TargetRegistry::RegisterAsmTargetStreamer(*T, createCpu0AsmTargetStreamer);
    TargetRegistry::RegisterMCAsmBackend(*T, createCpu0AsmBackend);
  }

  TargetRegistry::RegisterMCCodeEmitter(getTheCpu0Target(),
                                        createCpu0MCCodeEmitterEB);
  TargetRegistry::RegisterMCCodeEmitter(getTheCpu0elTarget(),
                                        createCpu0MCCodeEmitterEL);
}
