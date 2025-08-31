// OS-specific code
#include "Platform.h"

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#include <ShlObj.h>
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
    PWSTR savedGamesWideChars = NULL;

    SHGetKnownFolderPath(FOLDERID_SavedGames, KF_FLAG_CREATE, NULL, &savedGamesWideChars);
    if (savedGamesWideChars == NULL)
    {
        return "";
    }

    int savedGamesLen = WideCharToMultiByte(CP_UTF8, 0, savedGamesWideChars, -1, NULL, 0, NULL, NULL);
    if (savedGamesLen == 0)
    {
        CoTaskMemFree(savedGamesWideChars);
        return "";
    }

    char *savedGamesChars = new char[savedGamesLen];
    int conversionResult = WideCharToMultiByte(CP_UTF8, 0, savedGamesWideChars, -1, savedGamesChars, savedGamesLen, NULL, NULL);
    CoTaskMemFree(savedGamesWideChars);
    if (conversionResult == 0)
    {
        delete savedGamesChars;
        return "";
    }

    std::string savesPath = std::string(savedGamesChars) + "\\" + FOLDER_NAME + "\\";
    delete savedGamesChars;
    return savesPath;
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

// App-specific subfolder under 'AppData/Local' on Windows, XDG_CONFIG_HOME (~/.config) on Linux
std::string getConfigsPath()
{
#ifdef _WIN32
    PWSTR localAppDataWideChars = NULL;

    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataWideChars);
    if (localAppDataWideChars == NULL)
    {
        return "";
    }

    int localAppDataLen = WideCharToMultiByte(CP_UTF8, 0, localAppDataWideChars, -1, NULL, 0, NULL, NULL);
    if (localAppDataLen == 0)
    {
        CoTaskMemFree(localAppDataWideChars);
        return "";
    }

    char *localAppDataChars = new char[localAppDataLen];
    int conversionResult = WideCharToMultiByte(CP_UTF8, 0, localAppDataWideChars, -1, localAppDataChars, localAppDataLen, NULL, NULL);
    CoTaskMemFree(localAppDataWideChars);
    if (conversionResult == 0)
    {
        delete localAppDataChars;
        return "";
    }

    std::string configsPath = std::string(localAppDataChars) + "\\" + FOLDER_NAME + "\\";
    delete localAppDataChars;
    return configsPath;
#else
    std::string configHome;

    if (const char *configHomeChars = std::getenv("XDG_CONFIG_HOME"))
    {
        configHome = std::string(configHomeChars);
    }
    else
    {
        if (const char *userHomeChars = std::getenv("HOME"))
        {
            configHome = std::string(userHomeChars) + "/.config";
        }
        else
        {
            return "";
        }
    }

    std::string configsPath = configHome + "/" + FOLDER_NAME + "/";
    return configsPath;
#endif
}
