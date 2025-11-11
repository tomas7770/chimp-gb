#pragma once

#include <cstdint>

enum SchedulerEventType
{
    APU_FrameSequencerTick,
    APU_Channel0Tick,
    APU_Channel1Tick,
    APU_Channel2Tick,
    APU_Channel3Tick,
    PushAudioSample,
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
