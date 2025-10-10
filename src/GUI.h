#pragma once

#include <SDL.h>
#include "Config.h"
#include "LCD.h"

class GUI
{
public:
    GUI(Config *config, SDL_Window *windowSDL, SDL_Renderer *rendererSDL, SDL_Texture *textureSDL);
    GUI() = default;

    void processEvent(SDL_Event *eventSDL);
    void draw();
    void destroy();

private:
    constexpr static float SCREEN_RATIO = float(LCD::SCREEN_W) / float(LCD::SCREEN_H);

    Config *mConfig;
    SDL_Renderer *mRendererSDL;
    SDL_Texture *mTextureSDL;

    std::string mImguiIniFilename; // need to keep a char buffer persistently
};
