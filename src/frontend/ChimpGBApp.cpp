#include "ChimpGBApp.h"
#include "Platform.h"
#include "../../res/default.ini.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void gameboyDrawCallback(void *userdata);
void saveGameCallback(void *userdata);

ChimpGBApp::ChimpGBApp(std::string &filepath, bool debug)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        terminate(-1);
    }

    loadConfig();

    int windowWidth, windowHeight;
    loadStateData(&windowWidth, &windowHeight);
    mWindowSDL = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  windowWidth, windowHeight,
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
    SDL_RenderSetVSync(mRendererSDL, 1);

    mTextureSDL = SDL_CreateTexture(mRendererSDL, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, LCD::SCREEN_W, LCD::SCREEN_H);
    if (mTextureSDL == NULL)
    {
        std::cout << "Blit texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        terminate(-1);
    }

    setupAudio();

    createDataDirectories();

    mGUI = GUI(this, &mConfig, mWindowSDL, mRendererSDL, mTextureSDL);
    mDebug = debug;

    if (filepath != "")
    {
        loadRomFile(filepath);
    }

    setVideoParameters();
}

void ChimpGBApp::loadRomFile(std::string &filepath)
{
    std::ifstream dataStream(filepath, std::ios::binary | std::ios::ate);
    if (!dataStream.good())
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE,
                                 "Error loading requested ROM. Perhaps this file doesn't exist?", mWindowSDL);
        return;
    }
    auto size = dataStream.tellg();
    dataStream.seekg(0);

    try
    {
        Cartridge cart = Cartridge(dataStream, size);
        std::string romFilename = std::filesystem::path(filepath).filename().stem().string();
        loadCart(cart, romFilename);
        mGUI.showMenuBar = false;
    }
    catch (std::runtime_error err)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
    }
    catch (std::logic_error err)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
    }
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

    std::string statePath = getStatePath();
    if (statePath != "")
    {
        std::filesystem::create_directories(statePath);
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

void ChimpGBApp::loadStateData(int *windowWidth, int *windowHeight)
{
    // Recent files
    std::ifstream recentFilesStream(getStatePath() + RECENT_FILES_NAME);
    std::stringstream buffer;
    buffer << recentFilesStream.rdbuf();
    std::string inString = buffer.str();
    recentFiles.load(inString);

    // Window state
    *windowWidth = WINDOW_WIDTH;
    *windowHeight = WINDOW_HEIGHT;
    std::ifstream windowStateStream(getStatePath() + WINDOW_STATE_NAME);
    buffer = std::stringstream();
    buffer << windowStateStream.rdbuf();
    inString = buffer.str();
    auto sizeDelim = inString.find("x");
    if (sizeDelim != inString.npos)
    {
        try
        {
            int w = std::stoi(inString.substr(0, sizeDelim));
            int h = std::stoi(inString.substr(sizeDelim + 1, inString.length()));
            *windowWidth = w;
            *windowHeight = h;
        }
        catch (std::exception err)
        {
        }
    }
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

void ChimpGBApp::saveStateData()
{
    // Recent files
    std::string outString;
    recentFiles.save(outString);
    std::ofstream recentFilesStream(getStatePath() + RECENT_FILES_NAME);
    recentFilesStream << outString;

    // Window state
    if (!mConfig.fullscreen)
    {
        std::ofstream windowStateStream(getStatePath() + WINDOW_STATE_NAME);
        int w, h;
        SDL_GetWindowSize(mWindowSDL, &w, &h);
        outString = std::to_string(w) + "x" + std::to_string(h);
        windowStateStream << outString;
    }
}

void ChimpGBApp::setVideoParameters()
{
    if (mConfig.fullscreen)
    {
        if (mConfig.exclusiveFullscreen)
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
            SDL_SetWindowFullscreen(mWindowSDL, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
    }
    else
    {
        SDL_SetWindowFullscreen(mWindowSDL, 0);
    }
}

void ChimpGBApp::setupAudio()
{
    if (mAudioDevSDL)
        SDL_CloseAudioDevice(mAudioDevSDL);

    cyclesPerSample = double(Gameboy::CLOCK_RATE) / double(mConfig.audioSampleRate);
    SDL_AudioSpec desiredAudioSpec, obtainedAudioSpec;
    desiredAudioSpec.freq = mConfig.audioSampleRate;
    desiredAudioSpec.format = AUDIO_F32;
    desiredAudioSpec.channels = 2;
    desiredAudioSpec.samples = mConfig.audioBufferSize;
    desiredAudioSpec.callback = NULL;
    mAudioDevSDL = SDL_OpenAudioDevice(NULL, 0, &desiredAudioSpec, &obtainedAudioSpec, 0);
    if (mAudioDevSDL == 0)
    {
        std::cout << "Audio device could not be opened! SDL_Error: " << SDL_GetError() << std::endl;
        terminate(-1);
    }
    SDL_PauseAudioDevice(mAudioDevSDL, 0);

    mAudioTimeAccum = 0.0;
}

void gameboyDrawCallback(void *userdata)
{
    ChimpGBApp *app = (ChimpGBApp *)userdata;
    app->gameboyDraw();
}

void ChimpGBApp::gameboyDraw()
{
    auto pixels = mGameboy->getPixels();
    switch (mGameboy->getSystemType())
    {
    case Gameboy::SystemType::DMG:
        for (int i = 0; i < LCD::SCREEN_W * LCD::SCREEN_H; i++)
        {
            switch (pixels[i].dmg)
            {
            default:
            case LCD::DMGColor::White:
                mTexturePixels[i] = 0xFFFFFFFF;
                break;
            case LCD::DMGColor::LightGray:
                mTexturePixels[i] = 0x7F7F7FFF;
                break;
            case LCD::DMGColor::DarkGray:
                mTexturePixels[i] = 0x3F3F3FFF;
                break;
            case LCD::DMGColor::Black:
                mTexturePixels[i] = 0x000000FF;
                break;
            }
        }
        break;

    case Gameboy::SystemType::CGB:
        for (int i = 0; i < LCD::SCREEN_W * LCD::SCREEN_H; i++)
        {
            LCD::CGBColor color = pixels[i].cgb;
            int red = (255 * color.r) / 31;
            int green = (255 * color.g) / 31;
            int blue = (255 * color.b) / 31;
            mTexturePixels[i] = (red << 24) | (green << 16) | (blue << 8) | 0xFF;
        }
        break;

    default:
        break;
    }

    SDL_UpdateTexture(mTextureSDL, NULL, mTexturePixels, LCD::SCREEN_W * 4);
}

void ChimpGBApp::drawDisplay()
{
    mGUI.draw();
}

void mainLoopCallback(void *userdata)
{
    ChimpGBApp *app = (ChimpGBApp *)userdata;
    app->mainLoop();
}

void ChimpGBApp::startMainLoop()
{
    frameTimestamp = SDL_GetTicks64();
    mRunning = true;
    mSleepTimeAccum = 0.0;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainLoopCallback, this, 0, 1);
#else
    while (1)
    {
        mainLoop();
    }
#endif
}

void ChimpGBApp::mainLoop()
{
    if (!mRunning)
    {
        return terminate(0);
    }

    {
        while (SDL_PollEvent(&mEventSDL) != 0)
        {
            bool processGameInput = mGUI.processEvent(&mEventSDL);
            if (mEventSDL.type == SDL_QUIT)
            {
                doExit();
            }
            else if (mEventSDL.type == SDL_KEYDOWN)
            {
                auto scancode = mEventSDL.key.keysym.scancode;
                if (scancode == mConfig.keyToggleFullscreen)
                {
                    mConfig.fullscreen = !mConfig.fullscreen;
                    setVideoParameters();
                }
                else if (scancode == mConfig.keyShowMenuBar)
                {
                    mGUI.showMenuBar = !mGUI.showMenuBar;
                }
                else if (processGameInput && mGameboy != nullptr && !mPaused)
                {
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
                }
            }
            else if (mEventSDL.type == SDL_KEYUP && processGameInput && mGameboy != nullptr && !mPaused)
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

        if (mGameboy == nullptr || mPaused)
        {
            drawDisplay();
            return;
        }

        std::vector<float> audioSamples;
        bool generateAudio = !mFastForward || (SDL_GetQueuedAudioSize(mAudioDevSDL) <= mConfig.audioLatency * sizeof(float) * 2);
        // Audio volume is not perceived as a linear function of amplitude; try to make up for that
        float audioVolume = mConfig.audioVolume * mConfig.audioVolume;
        for (int i = 0; i < Gameboy::CYCLES_PER_FRAME; i++)
        {
            try
            {
                mGameboy->doCycle();
            }
            catch (std::runtime_error err)
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
                terminate(-1);
            }
            catch (std::logic_error err)
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
                terminate(-1);
            }

            if (!generateAudio)
            {
                continue;
            }

            if (mConfig.audioQuality == Config::AudioQuality::High)
            {
                mGameboy->computeAudioSamples();
                leftAudioSamples.push_back(mGameboy->getLeftAudioSample());
                rightAudioSamples.push_back(mGameboy->getRightAudioSample());
            }

            mAudioTimeAccum += 1.0;
            if (mAudioTimeAccum >= cyclesPerSample)
            {
                mAudioTimeAccum -= cyclesPerSample;

                float leftSample = 0.0F, rightSample = 0.0F;
                switch (mConfig.audioQuality)
                {
                case Config::AudioQuality::Low:
                    mGameboy->computeAudioSamples();
                    leftSample = mGameboy->getLeftAudioSample();
                    rightSample = mGameboy->getRightAudioSample();
                    break;

                default:
                case Config::AudioQuality::High:
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
                    break;
                }

                // Apply volume
                leftSample *= 0.5F * audioVolume;
                rightSample *= 0.5F * audioVolume;

                // Clip to ensure bugs don't cause loud audio
                if (leftSample < -1.0F)
                    leftSample = -1.0F;
                else if (leftSample > 1.0F)
                    leftSample = 1.0F;

                if (rightSample < -1.0F)
                    rightSample = -1.0F;
                else if (rightSample > 1.0F)
                    rightSample = 1.0F;

                audioSamples.push_back(leftSample);
                audioSamples.push_back(rightSample);
            }
        }
        SDL_QueueAudio(mAudioDevSDL, audioSamples.data(), audioSamples.size() * sizeof(float));
        drawDisplay();
        while (!mFastForward && (SDL_GetQueuedAudioSize(mAudioDevSDL) > mConfig.audioLatency * sizeof(float) * 2))
        {
            uint64_t deltaTime = SDL_GetTicks64() - frameTimestamp;
            mSleepTimeAccum -= deltaTime;
            mSleepTimeAccum += FRAME_TIME;
            uint64_t startTime = SDL_GetTicks64();
            if (mSleepTimeAccum < 0.0)
            {
                mSleepTimeAccum = 0.0;
            }
            mainSleep(mSleepTimeAccum * 1e6);
            mSleepTimeAccum -= SDL_GetTicks64() - startTime;
        }
        frameTimestamp = SDL_GetTicks64();
    }
}

bool ChimpGBApp::isPoweredOn()
{
    return mGameboy != nullptr;
}

void ChimpGBApp::powerOff()
{
    if (mGameboy != nullptr)
    {
        saveGameSafe();
        delete mGameboy;
        mGameboy = nullptr;
        mPaused = false;
        SDL_RenderSetVSync(mRendererSDL, 1);
    }
}

void ChimpGBApp::loadCart(Cartridge &cart, std::string &romFilename)
{
    powerOff();

#ifndef __EMSCRIPTEN__
    SDL_RenderSetVSync(mRendererSDL, 0);
#endif

    mRomFilename = romFilename;

    Gameboy::SystemType systemType = static_cast<Gameboy::SystemType>(mConfig.dmgGameEmulatedConsole);
    if (cart.getHeader().cgbFlag & (1 << 7))
    {
        // CGB game
        systemType = static_cast<Gameboy::SystemType>(mConfig.cgbGameEmulatedConsole);
    }

    try
    {
        mGameboy = new Gameboy(cart, mDebug, systemType);
        mGameboy->setDrawCallback(gameboyDrawCallback, this);
        mGameboy->getCart().setSaveCallback(saveGameCallback, this);

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
}

void ChimpGBApp::reset()
{
    if (mGameboy != nullptr)
    {
        Cartridge cart = Cartridge(mGameboy->getCart());
        loadCart(cart, mRomFilename);
    }
}

bool ChimpGBApp::isPaused()
{
    return mPaused;
}

void ChimpGBApp::pause()
{
    if (isPoweredOn())
    {
        mPaused = !mPaused;
#ifndef __EMSCRIPTEN__
        SDL_RenderSetVSync(mRendererSDL, mPaused ? 1 : 0);
#endif
    }
}

void saveGameCallback(void *userdata)
{
    ChimpGBApp *app = (ChimpGBApp *)userdata;
    app->saveGameSafe();
}

void ChimpGBApp::saveGameSafe()
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

void ChimpGBApp::saveGame()
{
    if (mGameboy == nullptr)
    {
        return;
    }

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
    if (mGameboy == nullptr)
    {
        return;
    }

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

void ChimpGBApp::doExit()
{
    mRunning = false;
}

void ChimpGBApp::terminate(int error_code)
{
    if (error_code == 0)
    {
        powerOff();
    }
    else if (mGameboy != nullptr)
    {
        delete mGameboy;
    }

    mGUI.destroy();

    if (error_code == 0)
    {
        try
        {
            saveConfig();
            saveStateData();
        }
        catch (std::exception err)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, WINDOW_TITLE, err.what(), mWindowSDL);
        }
    }

    if (mWindowSDL)
        SDL_DestroyWindow(mWindowSDL);
    if (mRendererSDL)
        SDL_DestroyRenderer(mRendererSDL);
    if (mAudioDevSDL)
        SDL_CloseAudioDevice(mAudioDevSDL);
    SDL_Quit();

#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#else
    std::exit(error_code);
#endif
}
