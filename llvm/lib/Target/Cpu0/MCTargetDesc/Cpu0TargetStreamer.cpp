//===-- Cpu0TargetStreamer.cpp - Cpu0 Target Streamer Methods -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Cpu0TargetStreamer.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

Cpu0TargetStreamer::Cpu0TargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

Cpu0TargetAsmStreamer::Cpu0TargetAsmStreamer(MCStreamer &S,
                                             formatted_raw_ostream &OS)
    : Cpu0TargetStreamer(S), OS(OS) {}
