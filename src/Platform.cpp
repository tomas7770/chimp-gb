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

constexpr char const *FOLDER_NAME = "ChimpGB";

// App-specific subfolder under 'Saved Games' on Windows, XDG_DATA_HOME (~/.local/share) on Linux
std::string getSavesPath()
{
#ifdef _WIN32
    // TODO
    return "";
#else
    std::string dataHome;

    if (const char *dataHomeChars = std::getenv("XDG_DATA_HOME"))
    {
        dataHome = std::string(dataHomeChars);
    }
    else
    {
        if (const char *userHomeChars = std::getenv("HOME"))
        {
            dataHome = std::string(userHomeChars) + "/.local/share";
        }
        else
        {
            return "";
        }
    }

    std::string savesPath = dataHome + "/" + FOLDER_NAME + "/";
    return savesPath;
#endif
}
