// OS-specific code
#include "Platform.h"

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <unistd.h>
#include <linux/limits.h>
#endif

void mainSleep(uint64_t ns)
{
#ifdef _WIN32
    Sleep(ns / 1e6);
#else
    usleep(ns / 1e3);
#endif
}
