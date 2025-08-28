#pragma once

#include <SDL.h>
#include <SDL_scancode.h>
#include "Gameboy.h"

class ChimpGBApp
{
public:
    ChimpGBApp(const Cartridge &cart, std::string &romFilename, bool debug);
    void mainLoop();

private:
    constexpr const static char *WINDOW_TITLE = "ChimpGB - Game Boy Emulator";
    constexpr static int WINDOW_WIDTH = 640;
    constexpr static int WINDOW_HEIGHT = 576;
    // SDL keys for Gameboy keys
    // TODO: make it configurable
    constexpr static SDL_Scancode KEYMAP[] = {
        // Order of joypad keys: Up, Down, Left, Right, A, B, Start, Select
        SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_RETURN, SDL_SCANCODE_RSHIFT,
    };
    SDL_Scancode FAST_FORWARD_KEY = SDL_SCANCODE_TAB;
    //
    constexpr static int AUDIO_SAMPLE_RATE = 44100;
    constexpr static uint16_t AUDIO_INTERNAL_BUFFER_SIZE = 1024;
    constexpr static int AUDIO_BUFFER_SIZE = 1024;
    constexpr static double CYCLES_PER_SAMPLE = double(Gameboy::CLOCK_RATE) / double(AUDIO_SAMPLE_RATE);

    constexpr const static char *SAVE_EXTENSION = ".sav";

    SDL_Window *mWindowSDL = NULL;
    SDL_Renderer *mRendererSDL = NULL;
    SDL_Texture *mTextureSDL = NULL;
    SDL_Event mEventSDL;
    SDL_AudioDeviceID mAudioDevSDL;
    uint32_t mTexturePixels[LCD::SCREEN_W * LCD::SCREEN_H];
    std::string mRomFilename;

    bool mFastForward = false;

    Gameboy *mGameboy = nullptr;

    void createDataDirectories();
    void drawDisplay();
    void saveGame();
    void loadGame();
    void terminate(int error_code);
};
