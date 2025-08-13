#pragma once

#include <SDL.h>
#include <SDL_scancode.h>
#include "Gameboy.h"

class ChimpGBApp
{
public:
    ChimpGBApp(const Cartridge &cart, bool debug);
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
    //

    SDL_Window *mWindowSDL = NULL;
    SDL_Renderer *mRendererSDL = NULL;
    SDL_Event mEventSDL;

    Gameboy *mGameboy = nullptr;

    void drawDisplay();
    void terminate(int error_code);
};
