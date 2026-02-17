#pragma once

const char *defaultIni =
    "; Keyboard keys assigned to each Game Boy button.\n"
    "; Each key is stored as an SDL keyboard scancode.\n"
    "; Possible values:\n"
    ";   https://wiki.libsdl.org/SDL2/SDL_Scancode\n"
    "[keysGame]\n"
    "up = 82\n"
    "down = 81\n"
    "left = 80\n"
    "right = 79\n"
    "a = 29\n"
    "b = 27\n"
    "start = 40\n"
    "select = 229\n"
    "\n"
    "; Keyboard keys assigned to various emulator functions.\n"
    "[keysEmulator]\n"
    "showMenuBar = 41\n"
    "fastForward = 43\n"
    "toggleFullscreen = 68\n"
    "\n"
    "[video]\n"
    "fullscreen = 0\n"
    "exclusiveFullscreen = 0\n"
    "integerScaling = 0\n"
    "uiScale = 1.0\n"
    "\n"
    "[audio]\n"
    "volume = 1.0\n"
    "mute = 0\n"
    "sampleRate = 44100\n"
    "; Internal audio buffer size, in samples.\n"
#ifdef __EMSCRIPTEN__
    "bufferSize = 512\n"
#else
    "bufferSize = 256\n"
#endif
    "; Audio latency, in samples.\n"
    "; If the audio feels delayed compared to video, decrease this value.\n"
    "; If the audio stutters or sounds distorted, increase this value.\n"
#ifdef __EMSCRIPTEN__
    "latency = 2048\n"
#else
    "latency = 1024\n"
#endif
    "; Audio sampling quality. Set to \"Low\" if you experience performance issues.\n"
    "; 0 = Low, 1 = High\n"
#ifdef __EMSCRIPTEN__
    "quality = 0\n"
#else
    "quality = 1\n"
#endif
    "\n"
    "[emulation]\n"
    "; Game Boy (monochrome) boot ROM path. This should be a 256 byte file.\n"
    "dmgBootRomPath =\n"
    "; Game Boy Color boot ROM path. This should be a 2304 byte file.\n"
    "cgbBootRomPath =\n"
    "; Which console to emulate when running a GB-only game.\n"
    "; 0 = DMG (Game Boy), 1 = CGB (Game Boy Color)\n"
    "dmgGameEmulatedConsole = 0\n"
    "; Which console to emulate when running a GBC-enhanced or GBC-only game.\n"
    "cgbGameEmulatedConsole = 1\n";
