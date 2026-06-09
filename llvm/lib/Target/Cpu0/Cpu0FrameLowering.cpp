#include "Cpu0FrameLowering.h"

Cpu0FrameLowering::Cpu0FrameLowering()
    : TargetFrameLowering(StackGrowsDown, Align(4), 0) {}