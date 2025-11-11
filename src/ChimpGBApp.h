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
    void setupAudio();
    void setAudioQuality(Config::AudioQuality quality);
    bool isPoweredOn();
    void powerOff();
    void reset();
    bool isPaused();
    void pause();
    void doExit();

    void startMainLoop();
    void mainLoop();
    void gameboyDraw();
    void pushAudioSample(const std::vector<float> &leftAudioSamples,
                         const std::vector<float> &rightAudioSamples);

private:
    constexpr const static char *WINDOW_TITLE = "ChimpGB - Game Boy Emulator";
    constexpr static int WINDOW_WIDTH = 640;
    constexpr static int WINDOW_HEIGHT = 576;

    constexpr static double FRAME_TIME = 1000.0 * double(Gameboy::CYCLES_PER_FRAME) / double(Gameboy::CLOCK_RATE);

    constexpr const static char *SAVE_EXTENSION = ".sav";

    constexpr const static char *RECENT_FILES_NAME = "recent_files";
    constexpr const static char *WINDOW_STATE_NAME = "window_state";

    SDL_Window *mWindowSDL = NULL;
    SDL_Renderer *mRendererSDL = NULL;
    SDL_Texture *mTextureSDL = NULL;
    SDL_Event mEventSDL;
    SDL_AudioDeviceID mAudioDevSDL;
    uint32_t mTexturePixels[LCD::SCREEN_W * LCD::SCREEN_H];
    std::string mRomFilename;

    double mCyclesPerSample;

    bool mRunning;
    bool mDebug;
    bool mPaused = false;
    bool mFastForward = false;
    uint64_t frameTimestamp;
    double mSleepTimeAccum;
    std::vector<float> mAudioSamples;

    Gameboy *mGameboy = nullptr;

    Config mConfig;

    GUI mGUI;

    void drawDisplay();
    void createDataDirectories();
    void loadConfig();
    void loadStateData(int *windowWidth, int *windowHeight);
    void saveConfig();
    void saveStateData();
    void loadCart(const Cartridge &cart, std::string &romFilename);
    void saveGame();
    void loadGame();
    void terminate(int error_code);
};
