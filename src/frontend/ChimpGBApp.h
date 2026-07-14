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
    void setTargetSpeed(float targetSpeed);
    bool isPoweredOn();
    void powerOff(int error_code = 0);
    void reset();
    bool isPaused();
    void pause();
    void doExit();

    void saveConfig();
    void saveGameSafe();

    void saveState();
    void loadState();

    void startMainLoop();
    void mainLoop();
    void gameboyDraw();
    void pushAudioSample(const std::vector<float> &leftAudioSamples,
                         const std::vector<float> &rightAudioSamples);

    constexpr static double BASE_FRAME_TIME = 1000.0 * double(Gameboy::CYCLES_PER_FRAME) / double(Gameboy::CLOCK_RATE);
    constexpr static double BASE_FRAME_RATE = 1000.0 / BASE_FRAME_TIME;

private:
    constexpr const static char *WINDOW_TITLE = "ChimpGB - Game Boy Emulator";
    constexpr static int WINDOW_WIDTH = 640;
    constexpr static int WINDOW_HEIGHT = 576;

    constexpr const static char *SAVE_EXTENSION = ".sav";
    constexpr const static char *SAVE_STATE_EXTENSION = ".ss0";

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
    uint64_t mCoreDeltaTime;
    double mSleepTimeAccum;
    std::vector<float> mAudioSamples;

    Gameboy *mGameboy = nullptr;

    Config mConfig;

    GUI mGUI;

    void drawDisplay();
    void createDataDirectories();
    void loadConfig();
    void loadStateData(int *windowWidth, int *windowHeight);
    void saveStateData();
    void loadCart(Cartridge &cart, std::string &romFilename);
    void saveGame();
    void loadGame();
    void terminate(int error_code);
};
