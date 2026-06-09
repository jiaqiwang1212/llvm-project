#ifndef LLVM_LIB_TARGET_CPU0_CPU0TARGETMACHINE_H
#define LLVM_LIB_TARGET_CPU0_CPU0TARGETMACHINE_H

// #include "MCTargetDesc/RISCVMCTargetDesc.h"
// #include "CPU0Subtarget.h"
#include "Cpu0Subtarget.h"
#include "Cpu0TargetObjectFile.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include <optional>

namespace llvm {
class Cpu0TargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF; // Cpu0 的 ELF 目标文件对象
  Cpu0Subtarget DefaultSubtarget;   // Cpu0 的默认子目标，包含了 Cpu0 的特性和功能，保存 CPU + feature 解析结果
  //   mutable StringMap<std::unique_ptr<Cpu0Subtarget>> SubtargetMap;

public:
  /*
    T:
    也就是Cpu0TargetInfo.cpp中注册的Target对象（是一个Static的Target），llvm会根据目标平台选择对应的TargetMachine
    TT: 目标平台的Triple，llvm会根据这个Triple选择对应的TargetMachine
    Cpu: Cpu0的cpu型号，llvm会根据这个cpu型号选择对应的TargetMachine,
    也就是cpu0el 或者cpu0 FS: 开关 subtarget feature的字符串 Options:
    ABI、MC、浮点、代码生成选项 RM: 决定重定位模型，静态链接、动态链接等
    CM：Code model，决定地址大小，small、medium、large等
    OL: 优化级别，O0、O1、O2、O3
    JIT: 是否是JIT编译
  */
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

  // 
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
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
