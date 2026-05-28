#ifndef LLVM_LIB_TARGET_CPU0_CPU0TARGETMACHINE_H
#define LLVM_LIB_TARGET_CPU0_CPU0TARGETMACHINE_H

// #include "MCTargetDesc/RISCVMCTargetDesc.h"
// #include "CPU0Subtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "Cpu0Subtarget.h"
#include <optional>

namespace llvm {
class Cpu0TargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  Cpu0Subtarget DefaultSubtarget;
  //   mutable StringMap<std::unique_ptr<Cpu0Subtarget>> SubtargetMap;

public:
  Cpu0TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT);

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  const Cpu0Subtarget *getSubtargetImpl() const { return &DefaultSubtarget; }
  const Cpu0Subtarget *getSubtargetImpl(const Function &) const override {
    return &DefaultSubtarget;
  }
};
// Cpu0 big endian target machine
class Cpu0ebTargetMachine : public Cpu0TargetMachine {
public:
  Cpu0ebTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                      bool JIT);
};

// Cpu0 little endian target machine
class Cpu0elTargetMachine : public Cpu0TargetMachine {
public:
  Cpu0elTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options, 
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                      bool JIT);
};

} // namespace llvm

#endif
