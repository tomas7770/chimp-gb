#pragma once

#include <SDL.h>
#include <SDL_scancode.h>
#include "Gameboy.h"
#include "Config.h"
#include "RecentFiles.h"
#include "GUI.h"

class ChimpGBApp
{
public:
    ChimpGBApp(std::string &filepath, bool debug);

    RecentFiles recentFiles;

    void loadRomFile(std::string &filepath);
    void setVideoParameters();
    bool isPoweredOn();
    void powerOff();
    void reset();
    bool isPaused();
    void pause();
    void doExit();

    void mainLoop();
    void gameboyDraw();

private:
    constexpr const static char *WINDOW_TITLE = "ChimpGB - Game Boy Emulator";
    constexpr static int WINDOW_WIDTH = 640;
    constexpr static int WINDOW_HEIGHT = 576;

    constexpr static int AUDIO_SAMPLE_RATE = 44100;
    constexpr static uint16_t AUDIO_INTERNAL_BUFFER_SIZE = 256;
    constexpr static int AUDIO_BUFFER_SIZE = 1024;
    constexpr static double CYCLES_PER_SAMPLE = double(Gameboy::CLOCK_RATE) / double(AUDIO_SAMPLE_RATE);
    constexpr static double FRAME_TIME = 1000.0 * double(Gameboy::CYCLES_PER_FRAME) / double(Gameboy::CLOCK_RATE);

    constexpr const static char *SAVE_EXTENSION = ".sav";

    constexpr const static char *RECENT_FILES_NAME = "recent_files";

    SDL_Window *mWindowSDL = NULL;
    SDL_Renderer *mRendererSDL = NULL;
    SDL_Texture *mTextureSDL = NULL;
    SDL_Event mEventSDL;
    SDL_AudioDeviceID mAudioDevSDL;
    uint32_t mTexturePixels[LCD::SCREEN_W * LCD::SCREEN_H];
    std::string mRomFilename;

    bool mRunning;
    bool mDebug;
    bool mPaused = false;
    bool mFastForward = false;

    Gameboy *mGameboy = nullptr;

    Config mConfig;

    GUI mGUI;

    void drawDisplay();
    void createDataDirectories();
    void loadConfig();
    void loadStateData();
    void saveConfig();
    void saveStateData();
    void loadCart(const Cartridge &cart, std::string &romFilename);
    void saveGame();
    void loadGame();
    void terminate(int error_code);
};
