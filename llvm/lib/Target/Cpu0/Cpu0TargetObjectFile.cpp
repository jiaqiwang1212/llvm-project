#include "Cpu0TargetMachine.h"
#include "Cpu0TargetObjectFile.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/DataLayout.h"
using namespace llvm;

static cl::opt<unsigned> SSThreshold(
    "cpu0-ssection-threshold", cl::Hidden,
    cl::desc("Small data and bss section threshold size (default=8)"),
    cl::init(8));

void Cpu0TargetObjectFile::Initialize(MCContext &Ctx, const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  InitializeELF(TM.Options.UseInitArray);

  // 创建两个特殊的section，分别用于存放小数据和小bss数据
  // .sdata: 存放小的已初始化数据，.sbss: 存放小的未初始化数据
  // 使用 GP 寄存器可以以更高效地访问这些小数据，因此它们被放在专门的段中
  // .sbss 使用0初始化整个段，而 .sdata 则包含实际的数据内容
  SmallDataSection = getContext().getELFSection(
      ".sdata", ELF::SHT_PROGBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);

  SmallBSSSection = getContext().getELFSection(".sbss", ELF::SHT_NOBITS,
                                               ELF::SHF_WRITE | ELF::SHF_ALLOC);

  this->TM = &static_cast<const Cpu0TargetMachine &>(TM);
}