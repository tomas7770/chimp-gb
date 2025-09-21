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

void loadStringKey(mINI::INIStructure &ini, std::string &dest, const char *section, const char *key)
{
    if (!ini.has(section) || !ini[section].has(key))
    {
        return;
    }
    dest = ini[section][key];
}

void Config::load(std::stringstream &configString)
{
    mINI::INIFile file(&configString);
    mINI::INIStructure ini;
    file.readbuffer(ini);

    loadIntKey(ini, keysGame[0], "keysGame", "right");
    loadIntKey(ini, keysGame[1], "keysGame", "left");
    loadIntKey(ini, keysGame[2], "keysGame", "up");
    loadIntKey(ini, keysGame[3], "keysGame", "down");
    loadIntKey(ini, keysGame[4], "keysGame", "a");
    loadIntKey(ini, keysGame[5], "keysGame", "b");
    loadIntKey(ini, keysGame[6], "keysGame", "select");
    loadIntKey(ini, keysGame[7], "keysGame", "start");

    loadIntKey(ini, keyFastForward, "keysEmulator", "fastForward");
    loadIntKey(ini, keyToggleFullscreen, "keysEmulator", "toggleFullscreen");

    loadIntKey(ini, fullscreen, "video", "fullscreen");

    loadStringKey(ini, dmgBootRomPath, "emulation", "dmgBootRomPath");
    loadStringKey(ini, cgbBootRomPath, "emulation", "cgbBootRomPath");
}

void Config::save(std::string &configFilepath)
{
    mINI::INIFile file(configFilepath);
    mINI::INIStructure ini;
    file.read(ini);

    ini["keysGame"]["right"] = std::to_string(keysGame[0]);
    ini["keysGame"]["left"] = std::to_string(keysGame[1]);
    ini["keysGame"]["up"] = std::to_string(keysGame[2]);
    ini["keysGame"]["down"] = std::to_string(keysGame[3]);
    ini["keysGame"]["a"] = std::to_string(keysGame[4]);
    ini["keysGame"]["b"] = std::to_string(keysGame[5]);
    ini["keysGame"]["select"] = std::to_string(keysGame[6]);
    ini["keysGame"]["start"] = std::to_string(keysGame[7]);

    ini["keysEmulator"]["fastForward"] = std::to_string(keyFastForward);
    ini["keysEmulator"]["toggleFullscreen"] = std::to_string(keyToggleFullscreen);

    ini["video"]["fullscreen"] = std::to_string(fullscreen);

    ini["emulation"]["dmgBootRomPath"] = dmgBootRomPath;
    ini["emulation"]["cgbBootRomPath"] = cgbBootRomPath;

    file.write(ini, true);
}
