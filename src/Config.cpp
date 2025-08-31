#include "Config.h"
#include "mini/ini.h"

void loadIntKey(mINI::INIStructure &ini, int &dest, const char *section, const char *key)
{
    if (!ini.has(section) || !ini[section].has(key))
    {
        return;
    }
    std::string &value = ini[section][key];
    try
    {
        dest = std::stoi(value);
    }
    catch (std::exception err)
    {
        // Ignore and keep existing value
    }
}

void Config::load(std::stringstream &configString)
{
    mINI::INIFile file(&configString);
    mINI::INIStructure ini;
    file.readbuffer(ini);

    loadIntKey(ini, keysGame[0], "keysGame", "up");
    loadIntKey(ini, keysGame[1], "keysGame", "down");
    loadIntKey(ini, keysGame[2], "keysGame", "left");
    loadIntKey(ini, keysGame[3], "keysGame", "right");
    loadIntKey(ini, keysGame[4], "keysGame", "a");
    loadIntKey(ini, keysGame[5], "keysGame", "b");
    loadIntKey(ini, keysGame[6], "keysGame", "start");
    loadIntKey(ini, keysGame[7], "keysGame", "select");

    loadIntKey(ini, keyFastForward, "keysEmulator", "fastForward");
}
