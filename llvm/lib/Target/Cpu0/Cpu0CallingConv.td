//===-- Cpu0CallingConv.td - Calling Conventions for Cpu0 --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Cpu0 architecture.
//===----------------------------------------------------------------------===//

/// CCIfSubtarget - Match if the current subtarget has a feature F.
class CCIfSubtarget<string F, CCAction A>:
  CCIf<!strconcat("State.getTarget().getSubtarget<Cpu0Subtarget>().", F), A>;

def CSR_O32 : CalleeSavedRegs<(add LR, FP,
                                   (sequence "S%u", 1, 0))>;

// Cpu0 32-bit return value convention.
def RetCC_Cpu0 : CallingConv<[
  // i32 return values go in V0, then V1.
  CCIfType<[i32], CCAssignToReg<[V0, V1]>>
]>;

// Cpu0 O32 ABI: first two i32 args in A0, A1; rest on stack.
def CC_Cpu0O32 : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[A0, A1]>>,
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def CC_Cpu0 : CallingConv<[
  CCDelegateTo<CC_Cpu0O32>
]>;
