#include "ChimpGBApp.h"
#include "Platform.h"
#include <iostream>

ChimpGBApp::ChimpGBApp(const Cartridge &cart, bool debug)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        terminate(-1);
    }

    mWindowSDL = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (mWindowSDL == NULL)
    {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        terminate(-1);
    }

    mRendererSDL = SDL_CreateRenderer(mWindowSDL, -1, SDL_RENDERER_ACCELERATED);
    if (mRendererSDL == NULL)
    {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        terminate(-1);
    }
    SDL_RenderSetLogicalSize(mRendererSDL, WINDOW_WIDTH, WINDOW_HEIGHT);

    try
    {
        mGameboy = new Gameboy(cart, debug);
    }
    catch (std::exception err)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
        terminate(-1);
    }
}

void ChimpGBApp::drawDisplay()
{
    // Clear screen
    SDL_SetRenderDrawColor(mRendererSDL, 0, 0, 0, 0xFF);
    SDL_RenderClear(mRendererSDL);

    // Draw display
    int xScale = WINDOW_WIDTH / LCD::SCREEN_W;
    int yScale = WINDOW_HEIGHT / LCD::SCREEN_H;
    auto pixels = mGameboy->getPixels();
    for (int i = 0; i < LCD::SCREEN_W * LCD::SCREEN_H; i++)
    {
        // Render filled quad
        SDL_Rect fillRect = {(i % LCD::SCREEN_W) * xScale, (i / LCD::SCREEN_W) * yScale, xScale, yScale};
        switch (pixels[i])
        {
        default:
        case LCD::Color::White:
            SDL_SetRenderDrawColor(mRendererSDL, 0xFF, 0xFF, 0xFF, 0xFF);
            break;
        case LCD::Color::LightGray:
            SDL_SetRenderDrawColor(mRendererSDL, 0x7F, 0x7F, 0x7F, 0xFF);
            break;
        case LCD::Color::DarkGray:
            SDL_SetRenderDrawColor(mRendererSDL, 0x3F, 0x3F, 0x3F, 0xFF);
            break;
        case LCD::Color::Black:
            SDL_SetRenderDrawColor(mRendererSDL, 0x00, 0x00, 0x00, 0xFF);
            break;
        }
        SDL_RenderFillRect(mRendererSDL, &fillRect);
    }

    // Update renderer
    SDL_RenderPresent(mRendererSDL);
}

void ChimpGBApp::mainLoop()
{
    uint64_t frameTimestamp = SDL_GetTicks64();
    bool running = true;
    while (running)
    {
        while (SDL_PollEvent(&mEventSDL) != 0)
        {
            if (mEventSDL.type == SDL_QUIT)
                running = false;
        }
        uint64_t deltaTime = SDL_GetTicks64() - frameTimestamp;
        frameTimestamp = SDL_GetTicks64();
        try
        {
            mGameboy->tick(1e6 * deltaTime);
        }
        catch (std::exception err)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
            terminate(-1);
        }
        drawDisplay();
        mainSleep(mGameboy->getFrameClockTimeLeft());
    }

    terminate(0);
}

void ChimpGBApp::terminate(int error_code)
{
    if (mWindowSDL)
        SDL_DestroyWindow(mWindowSDL);
    if (mRendererSDL)
        SDL_DestroyRenderer(mRendererSDL);
    SDL_Quit();
    if (mGameboy != nullptr)
        delete mGameboy;
    std::exit(error_code);
}
