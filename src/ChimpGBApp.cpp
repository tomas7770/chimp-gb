#include "ChimpGBApp.h"
#include "Platform.h"
#include "../res/default.ini.h"
#include <iostream>
#include <fstream>
#include <filesystem>

void drawDisplayCallback(void *userdata);

ChimpGBApp::ChimpGBApp(const Cartridge &cart, std::string &romFilename, bool debug)
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

    createDataDirectories();
    loadConfig();

    mRomFilename = std::move(romFilename);

    Gameboy::SystemType systemType = Gameboy::SystemType::DMG;
    if (cart.getHeader().cgbFlag & (1 << 7))
    {
        // CGB game
        systemType = Gameboy::SystemType::CGB;
    }

    try
    {
        mGameboy = new Gameboy(cart, debug, systemType);
        mGameboy->setDrawCallback(drawDisplayCallback, this);

        std::string bootRomPath;
        switch (systemType)
        {
        case Gameboy::SystemType::DMG:
            bootRomPath = mConfig.dmgBootRomPath;
            break;

        case Gameboy::SystemType::CGB:
            bootRomPath = mConfig.cgbBootRomPath;
            break;

        default:
            break;
        }
        std::ifstream bootRomDataStream(bootRomPath, std::ios::binary);
        if (bootRomDataStream.good())
        {
            mGameboy->setBootRom(bootRomDataStream);
        }
        else
        {
            mGameboy->simulateBootRom();
        }

        loadGame();
    }
    catch (std::exception err)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
        terminate(-1);
    }

    setFullscreen();
}

void ChimpGBApp::createDataDirectories()
{
    std::string savesPath = getSavesPath();
    if (savesPath != "")
    {
        std::filesystem::create_directories(savesPath);
    }

    std::string configsPath = getConfigsPath();
    if (configsPath != "")
    {
        std::filesystem::create_directories(configsPath);
    }
}

void ChimpGBApp::loadConfig()
{
    // Load default values
    std::stringstream defaultIniBuffer;
    defaultIniBuffer << defaultIni;
    mConfig.load(defaultIniBuffer);

    // Load user values, if available
    std::string configFilepath = getConfigsPath() + CONFIG_NAME;
    std::ifstream configFileStream(configFilepath);
    std::stringstream userIniBuffer;
    userIniBuffer << configFileStream.rdbuf();
    mConfig.load(userIniBuffer);
}

void ChimpGBApp::saveConfig()
{
    // Load default INI and modify it with config values.
    // If the default INI is updated, the user gets the updates.
    std::stringstream defaultIniBuffer;
    defaultIniBuffer << defaultIni;

    std::string configFilepath = getConfigsPath() + CONFIG_NAME;
    std::ofstream configFileStream(configFilepath, std::ios::trunc);
    configFileStream << defaultIniBuffer.rdbuf();
    configFileStream.close();

    mConfig.save(configFilepath);
}

void ChimpGBApp::setFullscreen()
{
    if (mConfig.fullscreen)
    {
        int displayIndex = SDL_GetWindowDisplayIndex(mWindowSDL);
        if (displayIndex < 0)
        {
            return;
        }
        SDL_DisplayMode currentMode;
        if (SDL_GetCurrentDisplayMode(displayIndex, &currentMode) != 0)
        {
            return;
        }
        if (SDL_SetWindowDisplayMode(mWindowSDL, &currentMode) != 0)
        {
            return;
        }
        SDL_SetWindowFullscreen(mWindowSDL, SDL_WINDOW_FULLSCREEN);
    }
    else
    {
        SDL_SetWindowFullscreen(mWindowSDL, 0);
    }
}

void drawDisplayCallback(void *userdata)
{
    ChimpGBApp *app = (ChimpGBApp *)userdata;
    app->drawDisplay();
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
    double audioTimeAccum = 0.0;
    double sleepTimeAccum = 0.0;
    std::vector<float> leftAudioSamples, rightAudioSamples;
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
                auto scancode = mEventSDL.key.keysym.scancode;
                for (int i = 0; i < 8; i++)
                {
                    if (scancode == mConfig.keysGame[i])
                    {
                        mGameboy->onKeyPress(i);
                        break;
                    }
                }
                if (scancode == mConfig.keyFastForward)
                {
                    mFastForward = true;
                }
                else if (scancode == mConfig.keyToggleFullscreen)
                {
                    mConfig.fullscreen = !mConfig.fullscreen;
                    setFullscreen();
                }
            }
            else if (mEventSDL.type == SDL_KEYUP)
            {
                for (int i = 0; i < 8; i++)
                {
                    if (mEventSDL.key.keysym.scancode == mConfig.keysGame[i])
                    {
                        mGameboy->onKeyRelease(i);
                        break;
                    }
                }
                if (mEventSDL.key.keysym.scancode == mConfig.keyFastForward)
                {
                    mFastForward = false;
                }
            }
        }
        std::vector<float> audioSamples;
        for (int i = 0; i < Gameboy::CYCLES_PER_FRAME; i++)
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

            if (!(mGameboy->tCycleCounter % Gameboy::APU_CYCLE_DIV))
            {
                mGameboy->computeAudioSamples();
                leftAudioSamples.push_back(mGameboy->getLeftAudioSample());
                rightAudioSamples.push_back(mGameboy->getRightAudioSample());
            }

            audioTimeAccum += 1.0;
            if (audioTimeAccum >= CYCLES_PER_SAMPLE)
            {
                audioTimeAccum -= CYCLES_PER_SAMPLE;

                float leftSample = 0.0F, rightSample = 0.0F;
                for (float sample : leftAudioSamples)
                {
                    leftSample += sample;
                }
                for (float sample : rightAudioSamples)
                {
                    rightSample += sample;
                }
                leftSample /= leftAudioSamples.size();
                rightSample /= rightAudioSamples.size();
                leftAudioSamples.clear();
                rightAudioSamples.clear();

                // Clip to ensure bugs don't cause loud audio
                if (leftSample < -1.0F)
                    leftSample = -1.0F;
                else if (leftSample > 1.0F)
                    leftSample = 1.0F;

                if (rightSample < -1.0F)
                    rightSample = -1.0F;
                else if (rightSample > 1.0F)
                    rightSample = 1.0F;

                // -1.0 to 1.0 is too loud so the range is decreased
                audioSamples.push_back(leftSample * 0.5F);
                audioSamples.push_back(rightSample * 0.5F);
            }
        }
        if (mFastForward)
        {
            SDL_ClearQueuedAudio(mAudioDevSDL);
        }
        SDL_QueueAudio(mAudioDevSDL, audioSamples.data(), audioSamples.size() * sizeof(float));
        while (SDL_GetQueuedAudioSize(mAudioDevSDL) > AUDIO_BUFFER_SIZE * sizeof(float) * 2)
        {
            uint64_t deltaTime = SDL_GetTicks64() - frameTimestamp;
            sleepTimeAccum -= deltaTime;
            sleepTimeAccum += FRAME_TIME;
            uint64_t startTime = SDL_GetTicks64();
            if (sleepTimeAccum < 0.0)
            {
                sleepTimeAccum = 0.0;
            }
            mainSleep(sleepTimeAccum * 1e6);
            sleepTimeAccum -= SDL_GetTicks64() - startTime;
        }
        frameTimestamp = SDL_GetTicks64();
    }

    terminate(0);
}

void ChimpGBApp::saveGame()
{
    const Cartridge &cart = mGameboy->getCart();
    if (cart.hasBattery())
    {
        std::string saveFilepath = getSavesPath() + mRomFilename + std::string(SAVE_EXTENSION);
        std::ofstream dataStream(saveFilepath, std::ios::binary | std::ios::trunc);

        const uint8_t *sram = cart.getSRAM();
        if (sram != nullptr)
        {
            int ramSize = cart.getHeader().ramSize;
            dataStream.write(reinterpret_cast<const char *>(sram), ramSize);
        }

        if (cart.hasClock())
        {
            const MBC::RTC *rtc = cart.getRTC();
            if (rtc != nullptr)
            {
                uint8_t rtcBytes[48] = {0};
                rtcBytes[0] = rtc->timeSeconds;
                rtcBytes[4] = rtc->timeMinutes;
                rtcBytes[8] = rtc->timeHours;
                rtcBytes[12] = rtc->timeDays;
                rtcBytes[16] = rtc->timeDaysHigh;
                rtcBytes[20] = rtc->latchedTimeSeconds;
                rtcBytes[24] = rtc->latchedTimeMinutes;
                rtcBytes[28] = rtc->latchedTimeHours;
                rtcBytes[32] = rtc->latchedTimeDays;
                rtcBytes[36] = rtc->latchedTimeDaysHigh;
                memcpy(rtcBytes + 40, &(rtc->timestamp), 8);
                dataStream.write(reinterpret_cast<char *>(rtcBytes), 48);
            }
        }
    }
}

void ChimpGBApp::loadGame()
{
    Cartridge &cart = mGameboy->getCart();
    if (cart.hasBattery())
    {
        std::string saveFilepath = getSavesPath() + mRomFilename + std::string(SAVE_EXTENSION);
        std::ifstream dataStream(saveFilepath, std::ios::binary);

        int ramSize = cart.getHeader().ramSize;
        cart.loadSRAM(dataStream, ramSize);

        if (cart.hasClock())
        {
            uint8_t rtcBytes[48];
            dataStream.read(reinterpret_cast<char *>(rtcBytes), 48);

            MBC::RTC rtcData;
            rtcData.timeSeconds = rtcBytes[0];
            rtcData.timeMinutes = rtcBytes[4];
            rtcData.timeHours = rtcBytes[8];
            rtcData.timeDays = rtcBytes[12];
            rtcData.timeDaysHigh = rtcBytes[16];
            rtcData.latchedTimeSeconds = rtcBytes[20];
            rtcData.latchedTimeMinutes = rtcBytes[24];
            rtcData.latchedTimeHours = rtcBytes[28];
            rtcData.latchedTimeDays = rtcBytes[32];
            rtcData.latchedTimeDaysHigh = rtcBytes[36];
            memcpy(&(rtcData.timestamp), rtcBytes + 40, 8);

            cart.loadRTC(rtcData);
        }
    }
}

void ChimpGBApp::terminate(int error_code)
{
    if (mGameboy != nullptr)
    {
        if (error_code == 0)
        {
            try
            {
                saveGame();
            }
            catch (std::exception err)
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
            }
        }
        delete mGameboy;
    }

    if (mWindowSDL)
        SDL_DestroyWindow(mWindowSDL);
    if (mRendererSDL)
        SDL_DestroyRenderer(mRendererSDL);
    if (mAudioDevSDL)
        SDL_CloseAudioDevice(mAudioDevSDL);
    SDL_Quit();

    if (error_code == 0)
    {
        try
        {
            saveConfig();
        }
        catch (std::exception err)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
        }
    }

    std::exit(error_code);
}
