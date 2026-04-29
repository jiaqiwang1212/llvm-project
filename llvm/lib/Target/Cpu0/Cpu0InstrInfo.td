//===- Cpu0InstrInfo.td - Target Description for Cpu0 Target -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Cpu0 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Cpu0 profiles and nodes
//===----------------------------------------------------------------------===//

def SDT_Cpu0Ret          : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
def SDT_Cpu0DivRem       : SDTypeProfile<0, 2, [SDTCisInt<0>, SDTCisSameAs<0,1>]>;

// Return
def Cpu0Ret : SDNode<"Cpu0ISD::Ret", SDT_Cpu0Ret,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Divide/Remainder nodes (result in HI/LO via glue; isel pattern on SDIV/UDIV)
def Cpu0DivRem  : SDNode<"Cpu0ISD::DivRem",  SDT_Cpu0DivRem, [SDNPOutGlue]>;
def Cpu0DivRemU : SDNode<"Cpu0ISD::DivRemU", SDT_Cpu0DivRem, [SDNPOutGlue]>;

//===----------------------------------------------------------------------===//
// Cpu0 Instruction Predicate Definitions.
//===----------------------------------------------------------------------===//

def Ch3_1       :     Predicate<"Subtarget->hasChapter3_1()">,
                      AssemblerPredicate<(all_of FeatureChapter3_1)>;
def Ch3_2       :     Predicate<"Subtarget->hasChapter3_2()">,
                      AssemblerPredicate<(all_of FeatureChapter3_2)>;
def Ch3_3       :     Predicate<"Subtarget->hasChapter3_3()">,
                      AssemblerPredicate<(all_of FeatureChapter3_3)>;
def Ch3_4       :     Predicate<"Subtarget->hasChapter3_4()">,
                      AssemblerPredicate<(all_of FeatureChapter3_4)>;
def Ch3_5       :     Predicate<"Subtarget->hasChapter3_5()">,
                      AssemblerPredicate<(all_of FeatureChapter3_5)>;
def Ch4_1       :     Predicate<"Subtarget->hasChapter4_1()">,
                      AssemblerPredicate<(all_of FeatureChapter4_1)>;
def Ch4_2       :     Predicate<"Subtarget->hasChapter4_2()">,
                      AssemblerPredicate<(all_of FeatureChapter4_2)>;
def Ch5_1       :     Predicate<"Subtarget->hasChapter5_1()">,
                      AssemblerPredicate<(all_of FeatureChapter5_1)>;
def Ch6_1       :     Predicate<"Subtarget->hasChapter6_1()">,
                      AssemblerPredicate<(all_of FeatureChapter6_1)>;
def Ch7_1       :     Predicate<"Subtarget->hasChapter7_1()">,
                      AssemblerPredicate<(all_of FeatureChapter7_1)>;
def Ch8_1       :     Predicate<"Subtarget->hasChapter8_1()">,
                      AssemblerPredicate<(all_of FeatureChapter8_1)>;
def Ch8_2       :     Predicate<"Subtarget->hasChapter8_2()">,
                      AssemblerPredicate<(all_of FeatureChapter8_2)>;
def Ch9_1       :     Predicate<"Subtarget->hasChapter9_1()">,
                      AssemblerPredicate<(all_of FeatureChapter9_1)>;
def Ch9_2       :     Predicate<"Subtarget->hasChapter9_2()">,
                      AssemblerPredicate<(all_of FeatureChapter9_2)>;
def Ch9_3       :     Predicate<"Subtarget->hasChapter9_3()">,
                      AssemblerPredicate<(all_of FeatureChapter9_3)>;
def Ch10_1      :     Predicate<"Subtarget->hasChapter10_1()">,
                      AssemblerPredicate<(all_of FeatureChapter10_1)>;
def Ch11_1      :     Predicate<"Subtarget->hasChapter11_1()">,
                      AssemblerPredicate<(all_of FeatureChapter11_1)>;
def Ch11_2      :     Predicate<"Subtarget->hasChapter11_2()">,
                      AssemblerPredicate<(all_of FeatureChapter11_2)>;
def Ch12_1      :     Predicate<"Subtarget->hasChapter12_1()">,
                      AssemblerPredicate<(all_of FeatureChapter12_1)>;
def Ch_all      :     Predicate<"Subtarget->hasChapterAll()">,
                      AssemblerPredicate<(all_of FeatureChapterAll)>;

def EnableOverflow  : Predicate<"Subtarget->enableOverflow()">;
def DisableOverflow : Predicate<"Subtarget->disableOverflow()">;

def HasCmp      :     Predicate<"Subtarget->hasCmp()">;
def HasSlt      :     Predicate<"Subtarget->hasSlt()">;

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "Cpu0InstrFormats.td"

//===----------------------------------------------------------------------===//
// Cpu0 Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//
// Instruction operand types

// Signed Operand
def simm16      : Operand<i32> {
  let DecoderMethod= "DecodeSimm16";
}

// Address operand
def mem : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPROut, simm16);
  let EncoderMethod = "getMemEncoding";
}

// Node immediate fits as 16-bit sign extended on target immediate.
// e.g. addi, andi
def immSExt16  : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

// Immediate whose low 16 bits are zero (upper-half load pattern for LUi).
def immLow16Zero : PatLeaf<(imm), [{
  uint64_t Val = N->getZExtValue();
  return (Val & 0xffffULL) == 0;
}]>;

// Zero-extended 16-bit immediate (matches 0..65535).
def immZExt16  : PatLeaf<(imm), [{ return isUInt<16>(N->getZExtValue()); }]>;

// Unsigned 16-bit immediate operand.
def uimm16 : Operand<i32> {
  let DecoderMethod = "DecodeSimm16";
}

// Unsigned 5-bit shift amount operand (for encoding).
def uimm5 : Operand<i32> {
  let DecoderMethod = "DecodeSimm16";
}

// PatLeaf matching a 5-bit unsigned shift amount (for pattern matching).
def immZExt5 : PatLeaf<(imm), [{ return isUInt<5>(N->getZExtValue()); }]>;

// Extract upper/lower 16 bits from a 32-bit immediate constant.
def HI16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((N->getZExtValue() >> 16) & 0xffff,
                                    SDLoc(N), MVT::i32);
}]>;

def LO16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() & 0xffff,
                                    SDLoc(N), MVT::i32);
}]>;

// Cpu0 Address Mode! SDNode frameindex could possibly be a match
// since load and store instructions from stack used it.
def addr :
  ComplexPattern<iPTR, 2, "SelectAddr", [frameindex]>;

//===----------------------------------------------------------------------===//
// Pattern fragment for load/store
//===----------------------------------------------------------------------===//

class AlignedLoad<PatFrag Node> :
  PatFrag<(ops node:$ptr), (Node node:$ptr), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  return LD->getMemoryVT().getSizeInBits()/8 <= LD->getAlign().value();
}]>;

class AlignedStore<PatFrag Node> :
  PatFrag<(ops node:$val, node:$ptr), (Node node:$val, node:$ptr), [{
  StoreSDNode *SD = cast<StoreSDNode>(N);
  return SD->getMemoryVT().getSizeInBits()/8 <= SD->getAlign().value();
}]>;

// Load/Store PatFrags.
def load_a          : AlignedLoad<load>;
def store_a         : AlignedStore<store>;

//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//

// Arithmetic and logical instructions with 2 register operands.
class ArithLogicI<bits<8> op, string instr_asm, SDNode OpNode,
                  Operand Od, PatLeaf imm_type, RegisterClass RC> :
  FL<op, (outs GPROut:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
     [(set GPROut:$ra, (OpNode RC:$rb, imm_type:$imm16))], IIAlu> {
  let isReMaterializable = 1;
}

class FMem<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin>: FL<op, outs, ins, asmstr, pattern, itin> {
  bits<20> addr;
  let Inst{19-16} = addr{19-16};
  let Inst{15-0}  = addr{15-0};
  let DecoderMethod = "DecodeMem";
}

// Memory Load/Store
let canFoldAsLoad = 1 in
class LoadM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
            Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs RC:$ra), (ins MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(set RC:$ra, (OpNode addr:$addr))], IILoad> {
  let isPseudo = Pseudo;
}

class StoreM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
             Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs), (ins RC:$ra, MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(OpNode RC:$ra, addr:$addr)], IIStore> {
  let isPseudo = Pseudo;
}

// 32-bit load.
class LoadM32<bits<8> op, string instr_asm, PatFrag OpNode,
                   bit Pseudo = 0>
  : LoadM<op, instr_asm, OpNode, GPROut, mem, Pseudo> {
}

// 32-bit store.
class StoreM32<bits<8> op, string instr_asm, PatFrag OpNode,
                    bit Pseudo = 0>
  : StoreM<op, instr_asm, OpNode, GPROut, mem, Pseudo> {
}

let isBranch=1, isTerminator=1, isBarrier=1, imm16=0, hasDelaySlot = 1,
    isIndirectBranch = 1 in
class JumpFR<bits<8> op, string instr_asm, RegisterClass RC>:
  FL<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [(brind RC:$ra)], IIBranch> {
  let rb = 0;
  let imm16 = 0;
}

// Return instruction
class RetBase<RegisterClass RC>: JumpFR<0x3c, "ret", RC> {
  let isReturn = 1;
  let isCodeGenOnly = 1;
  let hasCtrlDep = 1;
  let hasExtraSrcRegAllocReq = 1;
}

// 3-operand arithmetic/logic (R-type, FA format)
class ArithLogicR<bits<8> op, string instr_asm, SDNode OpNode,
                  InstrItinClass itin, RegisterClass RC, bit isComm = 0> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
  let shamt = 0;
  let isCommutable = isComm;
}

// Shift by immediate (R-type, FA format, rc=0)
class ShiftRotateI<bits<8> op, string instr_asm,
                   SDNode OpNode, PatLeaf imm_type, RegisterClass RC> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, uimm5:$shamt),
     !strconcat(instr_asm, "\t$ra, $rb, $shamt"),
     [(set GPROut:$ra, (OpNode RC:$rb, imm_type:$shamt))], IIAlu> {
  let rc = 0;
}

// Shift by register (R-type, FA format)
class ShiftRotateR<bits<8> op, string instr_asm, SDNode OpNode,
                   RegisterClass RC> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (OpNode RC:$rb, RC:$rc))], IIAlu> {
  let shamt = 0;
}

// Multiply: results go to HI/LO implicitly.
class Mult32<bits<8> op, string instr_asm, InstrItinClass itin> :
  FA<op, (outs), (ins CPURegs:$ra, CPURegs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], itin> {
  let rc = 0;
  let shamt = 0;
  let isCommutable = 1;
  let Defs = [HI, LO];
}

// Divide: results go to HI/LO implicitly; SDNode pattern for isel.
class Div32<SDNode opNode, bits<8> op, string instr_asm, InstrItinClass itin> :
  FA<op, (outs), (ins CPURegs:$ra, CPURegs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(opNode CPURegs:$ra, CPURegs:$rb)], itin> {
  let rc = 0;
  let shamt = 0;
  let Defs = [HI, LO];
}

// Move from HI or LO register
class MoveFromLOHI<bits<8> op, string instr_asm, RegisterClass RC,
                   list<Register> UseRegs> :
  FA<op, (outs RC:$ra), (ins),
     !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let rc = 0;
  let shamt = 0;
  let Uses = UseRegs;
}

// Move to HI or LO register
class MoveToLOHI<bits<8> op, string instr_asm, RegisterClass RC,
                 list<Register> DefRegs> :
  FA<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let rc = 0;
  let shamt = 0;
  let Defs = DefRegs;
}

class MoveToC0<bits<8> op, string instr_asm, RegisterClass RC> :
  FA<op, (outs C0Regs:$ra), (ins RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], IIAlu> {
  let rc = 0;
  let shamt = 0;
}

// Move from C0 register
class MoveFromC0<bits<8> op, string instr_asm, RegisterClass RC> :
  FA<op, (outs RC:$ra), (ins C0Regs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], IIAlu> {
  let rc = 0;
  let shamt = 0;
}

// Move between C0 registers
class C0Move<bits<8> op, string instr_asm> :
  FA<op, (outs C0Regs:$rb), (ins C0Regs:$ra),
     !strconcat(instr_asm, "\t$rb, $ra"), [], IIAlu> {
  let rc = 0;
  let shamt = 0;
}

//===----------------------------------------------------------------------===//
// Instruction definition
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Cpu0 Instructions
//===----------------------------------------------------------------------===//

/// Load and Store Instructions
///  aligned
def LD     : LoadM32<0x01,  "ld",  load_a>;
def ST     : StoreM32<0x02, "st",  store_a>;

/// Arithmetic Instructions (ALU Immediate)
// IR "add" defined in include/llvm/Target/TargetSelectionDAG.td, line 315 (def add).
def ADDiu   : ArithLogicI<0x09, "addiu", add, simm16, immSExt16, CPURegs>;

let Predicates = [Ch3_5] in {
def ORi     : ArithLogicI<0x0d, "ori", or, uimm16, immZExt16, CPURegs>;
}

let Predicates = [Ch3_5] in {
let Predicates = [DisableOverflow] in {
def ADDu    : ArithLogicR<0x11, "addu", add, IIAlu, GPROut, 1>;
}
}

/// Arithmetic Instructions (3-Operand, R-Type)
let Predicates = [Ch4_1] in {

// Integer arithmetic (overflow-controlled)
let Predicates = [DisableOverflow] in {
def SUBu    : ArithLogicR<0x12, "subu", sub, IIAlu, GPROut>;
}
let Predicates = [EnableOverflow] in {
def ADD     : ArithLogicR<0x13, "add",  add, IIAlu, GPROut, 1>;
def SUB     : ArithLogicR<0x14, "sub",  sub, IIAlu, GPROut>;
}
def MUL   : ArithLogicR<0x17, "mul",  mul, IImul,  GPROut, 1>;

// Bitwise logic
def AND   : ArithLogicR<0x18, "and",  and, IIAlu,  GPROut, 1>;
def OR    : ArithLogicR<0x19, "or",   or,  IIAlu,  GPROut, 1>;
def XOR   : ArithLogicR<0x1a, "xor",  xor, IIAlu,  GPROut, 1>;
// NOR has no standard LLVM SDNode; matched via Pat<(not (or ...)), NOR> below.
let isCommutable = 1 in
def NOR   : FA<0x1b, (outs GPROut:$ra), (ins GPROut:$rb, GPROut:$rc),
               "nor\t$ra, $rb, $rc", [], IIAlu> {
  let shamt = 0;
}

// Multiply/Divide (results in HI/LO)
def MULT  : Mult32<0x41, "mult",  IImul>;
def MULTu : Mult32<0x42, "multu", IImul>;
def SDIV  : Div32<Cpu0DivRem,  0x43, "div",  IIIdiv>;
def UDIV  : Div32<Cpu0DivRemU, 0x44, "divu", IIIdiv>;

// HI/LO register moves
def MFHI  : MoveFromLOHI<0x46, "mfhi", GPROut, [HI]>;
def MFLO  : MoveFromLOHI<0x47, "mflo", GPROut, [LO]>;
def MTHI  : MoveToLOHI<0x48, "mthi", CPURegs, [HI]>;
def MTLO  : MoveToLOHI<0x49, "mtlo", CPURegs, [LO]>;

// Set-less-than (cpu032II with HasSlt) — uses PatFrag, not SDNode
class SetCC_R<bits<8> op, string instr_asm, PatFrag Cond, RegisterClass RC> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (Cond RC:$rb, RC:$rc))], IIAlu> {
  let shamt = 0;
}

class SetCC_I<bits<8> op, string instr_asm, PatFrag Cond,
              Operand Od, PatLeaf imm_type, RegisterClass RC> :
  FL<op, (outs GPROut:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
     [(set GPROut:$ra, (Cond RC:$rb, imm_type:$imm16))], IIAlu>;

let Predicates = [Ch4_1, HasSlt] in {
  def SLT   : SetCC_R<0x28, "slt",   setlt,  GPROut>;
  def SLTu  : SetCC_R<0x29, "sltu",  setult, GPROut>;
  def SLTi  : SetCC_I<0x2a, "slti",  setlt,  simm16, immSExt16, GPROut>;
  def SLTiu : SetCC_I<0x2b, "sltiu", setult, simm16, immSExt16, GPROut>;
}

// CMP (cpu032I with HasCmp) — no pattern; lowering handled by ISelLowering
let Predicates = [Ch4_1, HasCmp] in {
  def CMP : FA<0x2c, (outs GPROut:$ra), (ins GPROut:$rb, GPROut:$rc),
               "cmp\t$ra, $rb, $rc", [], IIAlu> {
    let shamt = 0;
  }
}

// C0 register moves
def MFC0  : MoveFromC0<0x50, "mfc0", CPURegs>;
def MTC0  : MoveToC0  <0x51, "mtc0", CPURegs>;
def C0MOV : C0Move    <0x52, "c0mov">;

} // Predicates = [Ch4_1]

let Predicates = [Ch3_5] in
def LUi : FL<0x0f, (outs GPROut:$ra), (ins uimm16:$imm16),
             "lui\t$ra, $imm16", [], IIHiLo> {
  let rb = 0;
}

/// Shift Instructions
let Predicates = [Ch3_5] in
def SHL  : ShiftRotateI<0x1e, "shl",  shl,  immZExt5, GPROut>;

let Predicates = [Ch4_1] in {
def ROL  : ShiftRotateI<0x1c, "rol",  rotl, immZExt5, GPROut>;
def ROR  : ShiftRotateI<0x1d, "ror",  rotr, immZExt5, GPROut>;
def SHR  : ShiftRotateI<0x1f, "shr",  srl,  immZExt5, GPROut>;
def SRA  : ShiftRotateI<0x20, "sra",  sra,  immZExt5, GPROut>;
def SRAV : ShiftRotateR<0x21, "srav", sra,  GPROut>;
def SHLV : ShiftRotateR<0x22, "shlv", shl,  GPROut>;
def SHRV : ShiftRotateR<0x23, "shrv", srl,  GPROut>;
def ROLV : ShiftRotateR<0x24, "rolv", rotl, GPROut>;
def RORV : ShiftRotateR<0x25, "rorv", rotr, GPROut>;
} // Predicates = [Ch4_1]

def JR      : JumpFR<0x3c, "jr", GPROut>;

def RET     : RetBase<GPROut>;

def : Pat<(Cpu0Ret GPROut:$ra), (RET GPROut:$ra)>;

/// No operation
let addr=0 in
  def NOP   : FJ<0, (outs), (ins), "nop", [], IIAlu>;

//===----------------------------------------------------------------------===//
//  Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

// Small sign-extended immediates
def : Pat<(i32 immSExt16:$in), (ADDiu ZERO, imm:$in)>;

let Predicates = [Ch3_5] in {
// Zero-extended 16-bit immediate -> ORi ZERO, imm
def : Pat<(i32 immZExt16:$in), (ORi ZERO, imm:$in)>;
// Upper-half load: immediate with low 16 bits zero -> LUi
def : Pat<(i32 immLow16Zero:$in), (LUi (HI16 imm:$in))>;
// Arbitrary 32-bit immediate -> LUi upper + ORi lower
def : Pat<(i32 imm:$imm), (ORi (LUi (HI16 imm:$imm)), (LO16 imm:$imm))>;
}

// NOR: NOT of OR
let Predicates = [Ch4_1] in
def : Pat<(not (or GPROut:$rb, GPROut:$rc)), (NOR GPROut:$rb, GPROut:$rc)>;
