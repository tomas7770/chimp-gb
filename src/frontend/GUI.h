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

    constexpr static int SAVE_STATE_SLOT_COUNT = 10;

    bool processEvent(SDL_Event *eventSDL);
    void onRomLoad();
    void draw();
    void destroy();

    bool showMenuBar = true;
    bool saveStateExists[SAVE_STATE_SLOT_COUNT];

private:
    constexpr static float SCREEN_RATIO = float(LCD::SCREEN_W) / float(LCD::SCREEN_H);
    constexpr static const char *KEYS_GAME_NAMES[] = {"Right", "Left", "Up", "Down", "A", "B", "Select", "Start"};

    ChimpGBApp *mApp;
    Config *mConfig;
    SDL_Renderer *mRendererSDL;
    SDL_Texture *mTextureSDL;

    std::string mImguiIniFilename; // need to keep a char buffer persistently

    bool mShowControlsWindow = false;
    bool mShowSpeedWindow = false;
    bool mShowUIScaleWindow = false;
    bool mShowDMGPaletteWindow = false;

    bool mChangingKeybind = false;
    int mKeybindToChange;

    float mTargetSpeed;
    float mTargetFPS;
    float mUIScale;

    void loadRomFile(std::string &openFilenameString);
    void setAudioSampleRate(int audioSampleRate);
    void updateUIScale();
};
