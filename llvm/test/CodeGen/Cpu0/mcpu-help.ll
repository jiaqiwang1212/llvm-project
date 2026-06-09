; REQUIRES: cpu0-registered-target
; RUN: llc -march=cpu0 -mcpu=help 2>&1 | FileCheck %s

; CHECK: Available CPUs for this target:
; CHECK: cpu032I  - Select the cpu032I processor.
; CHECK: cpu032II - Select the cpu032II processor.
; CHECK: Available features for this target:
; CHECK: ch14_1   - Enable Chapter instructions..
; CHECK: o32      - Enable o32 ABI..
; CHECK: s32      - Enable s32 ABI..
