#pragma once

#include <cstdint>

enum SchedulerEventType
{
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
