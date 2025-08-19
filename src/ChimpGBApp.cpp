#include "ChimpGBApp.h"
#include "Platform.h"
#include <iostream>

ChimpGBApp::ChimpGBApp(const Cartridge &cart, bool debug)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
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
    SDL_RenderSetLogicalSize(mRendererSDL, LCD::SCREEN_W, LCD::SCREEN_H);

    mTextureSDL = SDL_CreateTexture(mRendererSDL, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, LCD::SCREEN_W, LCD::SCREEN_H);
    if (mTextureSDL == NULL)
    {
        std::cout << "Blit texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        terminate(-1);
    }

    SDL_AudioSpec desiredAudioSpec, obtainedAudioSpec;
    desiredAudioSpec.freq = AUDIO_SAMPLE_RATE;
    desiredAudioSpec.format = AUDIO_F32;
    desiredAudioSpec.channels = 2;
    desiredAudioSpec.samples = AUDIO_INTERNAL_BUFFER_SIZE;
    desiredAudioSpec.callback = NULL;
    mAudioDevSDL = SDL_OpenAudioDevice(NULL, 0, &desiredAudioSpec, &obtainedAudioSpec, 0);
    if (mAudioDevSDL == 0)
    {
        std::cout << "Audio device could not be opened! SDL_Error: " << SDL_GetError() << std::endl;
        terminate(-1);
    }
    SDL_PauseAudioDevice(mAudioDevSDL, 0);

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
        switch (pixels[i])
        {
        default:
        case LCD::Color::White:
            mTexturePixels[i] = 0xFFFFFFFF;
            break;
        case LCD::Color::LightGray:
            mTexturePixels[i] = 0x7F7F7FFF;
            break;
        case LCD::Color::DarkGray:
            mTexturePixels[i] = 0x3F3F3FFF;
            break;
        case LCD::Color::Black:
            mTexturePixels[i] = 0x000000FF;
            break;
        }
    }

    // Update renderer
    SDL_UpdateTexture(mTextureSDL, NULL, mTexturePixels, LCD::SCREEN_W * 4);
    SDL_RenderCopy(mRendererSDL, mTextureSDL, NULL, NULL);
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
            {
                running = false;
            }
            else if (mEventSDL.type == SDL_KEYDOWN)
            {
                for (int i = 0; i < 8; i++)
                {
                    if (mEventSDL.key.keysym.scancode == KEYMAP[i])
                    {
                        mGameboy->onKeyPress(i);
                        break;
                    }
                }
                if (mEventSDL.key.keysym.scancode == FAST_FORWARD_KEY)
                {
                    mFastForward = true;
                }
            }
            else if (mEventSDL.type == SDL_KEYUP)
            {
                for (int i = 0; i < 8; i++)
                {
                    if (mEventSDL.key.keysym.scancode == KEYMAP[i])
                    {
                        mGameboy->onKeyRelease(i);
                        break;
                    }
                }
                if (mEventSDL.key.keysym.scancode == FAST_FORWARD_KEY)
                {
                    mFastForward = false;
                }
            }
        }
        int i = 0, j = 0;
        std::vector<float> audioSamples;
        while (i < Gameboy::CYCLES_PER_FRAME)
        {
            try
            {
                mGameboy->doTCycle();
            }
            catch (std::runtime_error err)
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
                terminate(-1);
            }
            if (j == 0)
            {
                auto leftSample = mGameboy->getLeftAudioSample();
                auto rightSample = mGameboy->getRightAudioSample();

                // Clip to ensure bugs don't cause loud audio
                if (leftSample < -1.0F)
                    leftSample = -1.0F;
                else if (leftSample > 1.0F)
                    leftSample = 1.0F;

                if (rightSample < -1.0F)
                    rightSample = -1.0F;
                else if (rightSample > 1.0F)
                    rightSample = 1.0F;

                // TODO -1.0 to 1.0 is too loud so the range is decreased
                audioSamples.push_back(leftSample);
                audioSamples.push_back(rightSample);
            }
            i++;
            j = (j + 1) % (Gameboy::CLOCK_RATE / AUDIO_SAMPLE_RATE);
        }
        if (mFastForward)
        {
            SDL_ClearQueuedAudio(mAudioDevSDL);
        }
        SDL_QueueAudio(mAudioDevSDL, audioSamples.data(), audioSamples.size() * sizeof(float));
        drawDisplay();
        while (SDL_GetQueuedAudioSize(mAudioDevSDL) > AUDIO_BUFFER_SIZE * sizeof(float) * 2)
        {
            mainSleep(1e6);
        }
    }

    terminate(0);
}

void ChimpGBApp::terminate(int error_code)
{
    if (mWindowSDL)
        SDL_DestroyWindow(mWindowSDL);
    if (mRendererSDL)
        SDL_DestroyRenderer(mRendererSDL);
    if (mAudioDevSDL)
        SDL_CloseAudioDevice(mAudioDevSDL);
    SDL_Quit();
    if (mGameboy != nullptr)
        delete mGameboy;
    std::exit(error_code);
}
