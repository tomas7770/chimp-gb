#pragma once

#include <cstdint>

enum SchedulerEventType
{
    PPU_OAMScan_End,
    PPU_Draw_End,
    PPU_NewLine,
    PPU_DelayedVBlank,
    PPU_EarlyLYUpdate,
    APU_FrameSequencerTick,
    FinishFrame
};

struct SchedulerEvent
{
    SchedulerEventType type;
    uint64_t timestamp;
    bool operator<(const SchedulerEvent &rhs) const
    {
        return timestamp < rhs.timestamp;
    }
};
