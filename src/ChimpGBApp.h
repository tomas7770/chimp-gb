#pragma once

#include <SDL.h>
#include "Gameboy.h"

class ChimpGBApp
{
public:
    ChimpGBApp(const Cartridge &cart, bool debug);
    void mainLoop();

private:
    constexpr const static char *WINDOW_TITLE = "ChimpGB - Gameboy Emulator";
    constexpr static int WINDOW_WIDTH = 640;
    constexpr static int WINDOW_HEIGHT = 576;

    SDL_Window *mWindowSDL = NULL;
    SDL_Renderer *mRendererSDL = NULL;
    SDL_Event mEventSDL;

    Gameboy *mGameboy = nullptr;

    void drawDisplay();
    void terminate(int error_code);
};
