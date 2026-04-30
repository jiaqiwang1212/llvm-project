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

// Signed 16-bit immediate operand.
def simm16      : Operand<i32> {
  let DecoderMethod= "DecodeSimm16";
}

// Address operand
def mem : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPROut, simm16);
  let EncoderMethod = "getMemEncoding";
}

// Unsigned 16-bit immediate operand.
def uimm16 : Operand<i32> {
  let DecoderMethod = "DecodeSimm16";
}

// Unsigned 5-bit shift amount operand (for encoding).
def uimm5 : Operand<i32> {
  let DecoderMethod = "DecodeSimm16";
}

// Node immediate fits as 16-bit sign extended on target immediate.
def immSExt16  : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

// Immediate whose low 16 bits are zero (upper-half load pattern for LUi).
def immLow16Zero : PatLeaf<(imm), [{
  uint64_t Val = N->getZExtValue();
  return (Val & 0xffffULL) == 0;
}]>;

// Zero-extended 16-bit immediate (matches 0..65535).
def immZExt16  : PatLeaf<(imm), [{ return isUInt<16>(N->getZExtValue()); }]>;

// PatLeaf matching a 5-bit unsigned shift amount.
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

// Cpu0 Address Mode: SDNode frameindex can match since stack loads/stores use it.
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

def load_a  : AlignedLoad<load>;
def store_a : AlignedStore<store>;

//===----------------------------------------------------------------------===//
// Instruction class templates
//===----------------------------------------------------------------------===//

// Arithmetic/logic immediate (FL format): ra = rb op imm16
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

class LoadM32<bits<8> op, string instr_asm, PatFrag OpNode, bit Pseudo = 0>
  : LoadM<op, instr_asm, OpNode, GPROut, mem, Pseudo>;

class StoreM32<bits<8> op, string instr_asm, PatFrag OpNode, bit Pseudo = 0>
  : StoreM<op, instr_asm, OpNode, GPROut, mem, Pseudo>;

let isBranch=1, isTerminator=1, isBarrier=1, imm16=0, hasDelaySlot = 1,
    isIndirectBranch = 1 in
class JumpFR<bits<8> op, string instr_asm, RegisterClass RC>:
  FL<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [(brind RC:$ra)], IIBranch> {
  let rb = 0;
  let imm16 = 0;
}

class RetBase<RegisterClass RC>: JumpFR<0x3c, "ret", RC> {
  let isReturn = 1;
  let isCodeGenOnly = 1;
  let hasCtrlDep = 1;
  let hasExtraSrcRegAllocReq = 1;
}

// 3-operand arithmetic/logic (R-type, FA format): ra = rb op rc
class ArithLogicR<bits<8> op, string instr_asm, SDNode OpNode,
                  InstrItinClass itin, RegisterClass RC, bit isComm = 0> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
  let shamt = 0;
  let isCommutable = isComm;
}

// Shift by immediate (FA format, rc=0)
class ShiftRotateI<bits<8> op, string instr_asm,
                   SDNode OpNode, PatLeaf imm_type, RegisterClass RC> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, uimm5:$shamt),
     !strconcat(instr_asm, "\t$ra, $rb, $shamt"),
     [(set GPROut:$ra, (OpNode RC:$rb, imm_type:$shamt))], IIAlu> {
  let rc = 0;
}

// Shift by register (FA format)
class ShiftRotateR<bits<8> op, string instr_asm, SDNode OpNode,
                   RegisterClass RC> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (OpNode RC:$rb, RC:$rc))], IIAlu> {
  let shamt = 0;
}

// Multiply: results stored implicitly in HI/LO.
class Mult32<bits<8> op, string instr_asm, InstrItinClass itin> :
  FA<op, (outs), (ins CPURegs:$ra, CPURegs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], itin> {
  let rc = 0;
  let shamt = 0;
  let isCommutable = 1;
  let Defs = [HI, LO];
}

// Divide: results stored implicitly in HI/LO; SDNode pattern drives isel.
class Div32<SDNode opNode, bits<8> op, string instr_asm, InstrItinClass itin> :
  FA<op, (outs), (ins CPURegs:$ra, CPURegs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(opNode CPURegs:$ra, CPURegs:$rb)], itin> {
  let rc = 0;
  let shamt = 0;
  let Defs = [HI, LO];
}

class MoveFromLOHI<bits<8> op, string instr_asm, RegisterClass RC,
                   list<Register> UseRegs> :
  FA<op, (outs RC:$ra), (ins),
     !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let rc = 0;
  let shamt = 0;
  let Uses = UseRegs;
}

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

class MoveFromC0<bits<8> op, string instr_asm, RegisterClass RC> :
  FA<op, (outs RC:$ra), (ins C0Regs:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], IIAlu> {
  let rc = 0;
  let shamt = 0;
}

class C0Move<bits<8> op, string instr_asm> :
  FA<op, (outs C0Regs:$rb), (ins C0Regs:$ra),
     !strconcat(instr_asm, "\t$rb, $ra"), [], IIAlu> {
  let rc = 0;
  let shamt = 0;
}

// Compare instruction — writes result into SW (status word register, SR class).
// cpu032I variant uses CMP/CMPu; cpu032II uses SLT family instead.
class CmpInstr<bits<8> op, string instr_asm,
               InstrItinClass itin, RegisterClass RC, RegisterClass RD,
               bit isComm = 0> :
  FA<op, (outs RD:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"), [], itin> {
  let shamt = 0;
  let isCommutable = isComm;
}

// Logical NOR: ra = ~(rb | rc).  Pattern embeds the not-of-or matching.
class LogicNOR<bits<8> op, string instr_asm, RegisterClass RC> :
  FA<op, (outs RC:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set RC:$ra, (not (or RC:$rb, RC:$rc)))], IIAlu> {
  let shamt = 0;
  let isCommutable = 1;
}

// SetCC register-register (cpu032II, requires HasSlt predicate).
class SetCC_R<bits<8> op, string instr_asm, PatFrag cond_op,
              RegisterClass RC> :
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (cond_op RC:$rb, RC:$rc))], IIAlu>,
  Requires<[HasSlt]> {
  let shamt = 0;
}

// SetCC register-immediate (cpu032II, requires HasSlt predicate).
class SetCC_I<bits<8> op, string instr_asm, PatFrag cond_op,
              Operand Od, PatLeaf imm_type, RegisterClass RC> :
  FL<op, (outs GPROut:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
     [(set GPROut:$ra, (cond_op RC:$rb, imm_type:$imm16))], IIAlu>,
  Requires<[HasSlt]>;

//===----------------------------------------------------------------------===//
// Instruction definitions
//===----------------------------------------------------------------------===//

/// Load and Store Instructions
def LD  : LoadM32<0x01, "ld",  load_a>;
def ST  : StoreM32<0x02, "st", store_a>;

/// Arithmetic Instructions (ALU Immediate)
def ADDiu : ArithLogicI<0x09, "addiu", add, simm16, immSExt16, CPURegs>;

let Predicates = [Ch3_5] in
def ORi   : ArithLogicI<0x0d, "ori",  or,  uimm16, immZExt16, CPURegs>;

let Predicates = [Ch4_2] in {
def ANDi  : ArithLogicI<0x0c, "andi", and, uimm16, immZExt16, CPURegs>;
def XORi  : ArithLogicI<0x0e, "xori", xor, uimm16, immZExt16, CPURegs>;
}

let Predicates = [Ch3_5] in {
let Predicates = [DisableOverflow] in {
def ADDu  : ArithLogicR<0x11, "addu", add, IIAlu, GPROut, 1>;
}
}

/// Arithmetic Instructions (3-Operand R-Type) — Chapter 4.1
let Predicates = [Ch4_1] in {

let Predicates = [DisableOverflow] in {
def SUBu  : ArithLogicR<0x12, "subu", sub, IIAlu, GPROut>;
}
let Predicates = [EnableOverflow] in {
def ADD   : ArithLogicR<0x13, "add",  add, IIAlu, GPROut, 1>;
def SUB   : ArithLogicR<0x14, "sub",  sub, IIAlu, GPROut>;
}
def MUL   : ArithLogicR<0x17, "mul",  mul, IImul,  GPROut, 1>;

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

// C0 register moves (processor control registers)
def MFC0  : MoveFromC0<0x50, "mfc0", CPURegs>;
def MTC0  : MoveToC0  <0x51, "mtc0", CPURegs>;
def C0MOV : C0Move    <0x52, "c0mov">;

} // Predicates = [Ch4_1]

/// Bitwise Logic Instructions — Chapter 4.2
let Predicates = [Ch4_2] in {
def AND   : ArithLogicR<0x18, "and", and, IIAlu, GPROut, 1>;
def OR    : ArithLogicR<0x19, "or",  or,  IIAlu, GPROut, 1>;
def XOR   : ArithLogicR<0x1a, "xor", xor, IIAlu, GPROut, 1>;
def NOR   : LogicNOR<0x1b, "nor", GPROut>;
}

/// Load Upper Immediate
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

/// Compare and Set-Less-Than Instructions — Chapter 4.2
// SLTi/SLTiu/SLT/SLTu are cpu032II variant (HasSlt).
// CMP/CMPu are cpu032I variant (HasCmp); they write to SW via SR register class.
let Predicates = [Ch4_2, HasSlt] in {
def SLTi  : SetCC_I<0x26, "slti",  setlt,  simm16, immSExt16, CPURegs>;
def SLTiu : SetCC_I<0x27, "sltiu", setult, simm16, immSExt16, CPURegs>;
def SLT   : SetCC_R<0x28, "slt",   setlt,  CPURegs>;
def SLTu  : SetCC_R<0x29, "sltu",  setult, CPURegs>;
}

let Predicates = [Ch4_2, HasCmp] in {
def CMP   : CmpInstr<0x2A, "cmp",  IIAlu, CPURegs, SR, 0>;
def CMPu  : CmpInstr<0x2B, "cmpu", IIAlu, CPURegs, SR, 0>;
}

def JR  : JumpFR<0x3c, "jr", GPROut>;
def RET : RetBase<GPROut>;

def : Pat<(Cpu0Ret GPROut:$ra), (RET GPROut:$ra)>;

/// No operation
let addr=0 in
  def NOP : FJ<0, (outs), (ins), "nop", [], IIAlu>;

//===----------------------------------------------------------------------===//
//  Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

// Small sign-extended immediates
def : Pat<(i32 immSExt16:$in), (ADDiu ZERO, imm:$in)>;

let Predicates = [Ch3_5] in {
def : Pat<(i32 immZExt16:$in),   (ORi ZERO, imm:$in)>;
def : Pat<(i32 immLow16Zero:$in), (LUi (HI16 imm:$in))>;
def : Pat<(i32 imm:$imm),        (ORi (LUi (HI16 imm:$imm)), (LO16 imm:$imm))>;
}

// not(x) = NOR(x, ZERO) — 1's complement of a single register.
let Predicates = [Ch4_2] in
def : Pat<(not CPURegs:$in), (NOR CPURegs:$in, ZERO)>;

//===----------------------------------------------------------------------===//
// setcc patterns — Chapter 4.2
//===----------------------------------------------------------------------===//

let Predicates = [Ch4_2] in {

// --- CMP variant (cpu032I) ---

// a == b  →  (CMP a, b) bit[1] set, shift right by 1 and AND 1
multiclass SeteqPatsCmp<RegisterClass RC> {
  def : Pat<(seteq RC:$lhs, RC:$rhs),
            (SHR (ANDi (CMP RC:$lhs, RC:$rhs), 2), 1)>;
  def : Pat<(setne RC:$lhs, RC:$rhs),
            (XORi (SHR (ANDi (CMP RC:$lhs, RC:$rhs), 2), 1), 1)>;
}

// a < b  →  (CMP a, b) bit[0] set
multiclass SetltPatsCmp<RegisterClass RC> {
  def : Pat<(setlt RC:$lhs, RC:$rhs),
            (ANDi (CMP RC:$lhs, RC:$rhs), 1)>;
  def : Pat<(setult RC:$lhs, RC:$rhs),
            (ANDi (CMPu RC:$lhs, RC:$rhs), 1)>;
}

// a <= b  ≡  !(b < a)
multiclass SetlePatsCmp<RegisterClass RC> {
  def : Pat<(setle RC:$lhs, RC:$rhs),
            (XORi (ANDi (CMP RC:$rhs, RC:$lhs), 1), 1)>;
  def : Pat<(setule RC:$lhs, RC:$rhs),
            (XORi (ANDi (CMPu RC:$rhs, RC:$lhs), 1), 1)>;
}

// a > b  ≡  b < a
multiclass SetgtPatsCmp<RegisterClass RC> {
  def : Pat<(setgt RC:$lhs, RC:$rhs),
            (ANDi (CMP RC:$rhs, RC:$lhs), 1)>;
  def : Pat<(setugt RC:$lhs, RC:$rhs),
            (ANDi (CMPu RC:$rhs, RC:$lhs), 1)>;
}

// a >= b  ≡  !(a < b)
multiclass SetgePatsCmp<RegisterClass RC> {
  def : Pat<(setge RC:$lhs, RC:$rhs),
            (XORi (ANDi (CMP RC:$lhs, RC:$rhs), 1), 1)>;
  def : Pat<(setuge RC:$lhs, RC:$rhs),
            (XORi (ANDi (CMPu RC:$lhs, RC:$rhs), 1), 1)>;
}

// --- SLT variant (cpu032II) ---

// a == b  →  SLTiu(a XOR b, 1)
// a != b  →  SLTu(ZERO, a XOR b)
multiclass SeteqPatsSlt<RegisterClass RC, Instruction SLTiuOp,
                        Instruction XOROp, Instruction SLTuOp,
                        Register ZEROReg> {
  def : Pat<(seteq RC:$lhs, RC:$rhs),
            (SLTiuOp (XOROp RC:$lhs, RC:$rhs), 1)>;
  def : Pat<(setne RC:$lhs, RC:$rhs),
            (SLTuOp ZEROReg, (XOROp RC:$lhs, RC:$rhs))>;
}

// a <= b  ≡  !(b < a)
multiclass SetlePatsSlt<RegisterClass RC,
                        Instruction SLTOp, Instruction SLTuOp> {
  def : Pat<(setle RC:$lhs, RC:$rhs),
            (XORi (SLTOp RC:$rhs, RC:$lhs), 1)>;
  def : Pat<(setule RC:$lhs, RC:$rhs),
            (XORi (SLTuOp RC:$rhs, RC:$lhs), 1)>;
}

// a > b  ≡  b < a
multiclass SetgtPatsSlt<RegisterClass RC,
                        Instruction SLTOp, Instruction SLTuOp> {
  def : Pat<(setgt RC:$lhs, RC:$rhs),
            (SLTOp RC:$rhs, RC:$lhs)>;
  def : Pat<(setugt RC:$lhs, RC:$rhs),
            (SLTuOp RC:$rhs, RC:$lhs)>;
}

// a >= b  ≡  !(a < b)
multiclass SetgePatsSlt<RegisterClass RC,
                        Instruction SLTOp, Instruction SLTuOp> {
  def : Pat<(setge RC:$lhs, RC:$rhs),
            (XORi (SLTOp RC:$lhs, RC:$rhs), 1)>;
  def : Pat<(setuge RC:$lhs, RC:$rhs),
            (XORi (SLTuOp RC:$lhs, RC:$rhs), 1)>;
}

// a >= imm  ≡  !(a < imm)
multiclass SetgeImmPatsSlt<RegisterClass RC,
                           Instruction SLTiOp, Instruction SLTiuOp> {
  def : Pat<(setge RC:$lhs, immSExt16:$rhs),
            (XORi (SLTiOp RC:$lhs, immSExt16:$rhs), 1)>;
  def : Pat<(setuge RC:$lhs, immSExt16:$rhs),
            (XORi (SLTiuOp RC:$lhs, immSExt16:$rhs), 1)>;
}

// Instantiate SLT variant patterns (cpu032II)
let Predicates = [HasSlt] in {
defm : SeteqPatsSlt<CPURegs, SLTiu, XOR, SLTu, ZERO>;
defm : SetlePatsSlt<CPURegs, SLT, SLTu>;
defm : SetgtPatsSlt<CPURegs, SLT, SLTu>;
defm : SetgePatsSlt<CPURegs, SLT, SLTu>;
defm : SetgeImmPatsSlt<CPURegs, SLTi, SLTiu>;
}

// Instantiate CMP variant patterns (cpu032I)
let Predicates = [HasCmp] in {
defm : SeteqPatsCmp<CPURegs>;
defm : SetltPatsCmp<CPURegs>;
defm : SetlePatsCmp<CPURegs>;
defm : SetgtPatsCmp<CPURegs>;
defm : SetgePatsCmp<CPURegs>;
}

} // let Predicates = [Ch4_2]
