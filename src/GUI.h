#pragma once

#include <SDL.h>
#include "Config.h"
#include "LCD.h"

class ChimpGBApp;

class GUI
{
public:
    GUI(ChimpGBApp *app, Config *config, SDL_Window *windowSDL, SDL_Renderer *rendererSDL, SDL_Texture *textureSDL);
    GUI() = default;

    bool processEvent(SDL_Event *eventSDL);
    void draw();
    void destroy();

    bool showMenuBar = true;

private:
    constexpr static float SCREEN_RATIO = float(LCD::SCREEN_W) / float(LCD::SCREEN_H);

    ChimpGBApp *mApp;
    Config *mConfig;
    SDL_Renderer *mRendererSDL;
    SDL_Texture *mTextureSDL;

    std::string mImguiIniFilename; // need to keep a char buffer persistently
};
