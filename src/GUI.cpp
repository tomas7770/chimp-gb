#include "GUI.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

#include "tinyfiledialogs/tinyfiledialogs.h"

#include "Platform.h"
#include "ChimpGBApp.h"

#include <filesystem>
#include <format>

GUI::GUI(ChimpGBApp *app, Config *config, SDL_Window *windowSDL, SDL_Renderer *rendererSDL, SDL_Texture *textureSDL)
{
    mApp = app;
    mConfig = config;
    mRendererSDL = rendererSDL;
    mTextureSDL = textureSDL;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    mImguiIniFilename = getConfigsPath() + "imgui.ini";
    io.IniFilename = mImguiIniFilename.c_str();
    ImGui::StyleColorsDark();
    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(mConfig->uiScale);
    style.FontScaleDpi = mConfig->uiScale;
    ImGui_ImplSDL2_InitForSDLRenderer(windowSDL, mRendererSDL);
    ImGui_ImplSDLRenderer2_Init(mRendererSDL);
}

bool GUI::processEvent(SDL_Event *eventSDL)
{
    ImGui_ImplSDL2_ProcessEvent(eventSDL);
    ImGuiIO &io = ImGui::GetIO();
    return !io.WantCaptureKeyboard;
}

void GUI::draw()
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGuiIO &io = ImGui::GetIO();

    {
        if (mApp->isPoweredOn())
        {
            float windowRatio = io.DisplaySize.x / io.DisplaySize.y;
            ImVec2 viewportSize;
            if (mConfig->integerScaling)
            {
                int scaleFactor = windowRatio >= SCREEN_RATIO ? int(io.DisplaySize.y / LCD::SCREEN_H) : int(io.DisplaySize.x / LCD::SCREEN_W);
                viewportSize = ImVec2(float(scaleFactor * LCD::SCREEN_W), float(scaleFactor * LCD::SCREEN_H));
            }
            else
            {
                if (windowRatio >= SCREEN_RATIO)
                {
                    viewportSize = ImVec2(io.DisplaySize.x * SCREEN_RATIO / windowRatio, io.DisplaySize.y);
                }
                else
                {
                    viewportSize = ImVec2(io.DisplaySize.x, io.DisplaySize.y * windowRatio / SCREEN_RATIO);
                }
            }
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2.0F - viewportSize.x / 2.0F,
                                           io.DisplaySize.y / 2.0F - viewportSize.y / 2.0F));
            ImGui::SetNextWindowSize(viewportSize);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
            ImGui::Begin("Game", nullptr,
                         ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_NoInputs);
            ImGui::Image((ImTextureID)(intptr_t)mTextureSDL, viewportSize);
            ImGui::PopStyleVar();
            ImGui::End();
        }
    }

    {
        if (showMenuBar)
        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Load ROM"))
                    {
                        char const *filterPatterns[2] = {"*.gb", "*.gbc"};
                        char *openFilename = tinyfd_openFileDialog("Load ROM", mApp->recentFiles.lastOpenLocation.c_str(),
                                                                   2, filterPatterns, "Game Boy ROM files", 0);
                        if (openFilename)
                        {
                            std::string openFilenameString(openFilename);
                            loadRomFile(openFilenameString);
                        }
                    }
                    // Constrain recent files menu size so it doesn't cover other menus,
                    // since file names can get quite long.
                    ImGui::SetNextWindowSizeConstraints(
                        {0.0F, -1},
                        {io.DisplaySize.x - ImGui::GetContentRegionAvail().x - 25.0F * mConfig->uiScale, -1});
                    if (ImGui::BeginMenu("Load recent"))
                    {
                        RecentFiles &recentFiles = mApp->recentFiles;
                        std::string selectedRom = "";
                        for (int i = 0; i < recentFiles.recentFiles.size(); i++)
                        {
                            std::string filename = std::filesystem::path(recentFiles.recentFiles.at(i)).filename().string();
                            std::string itemString = filename + "##" + std::to_string(i);
                            if (ImGui::MenuItem(itemString.c_str()))
                            {
                                selectedRom = recentFiles.recentFiles.at(i);
                            }
                            ImGui::SetItemTooltip("%s", recentFiles.recentFiles.at(i).c_str());
                        }
                        for (int i = recentFiles.recentFiles.size(); i < RecentFiles::MAX_RECENT_FILES; i++)
                        {
                            std::string itemString = "------##" + std::to_string(i);
                            if (ImGui::MenuItem(itemString.c_str(), nullptr, false, false))
                            {
                            }
                        }
                        if (selectedRom != "")
                        {
                            loadRomFile(selectedRom);
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Clear"))
                        {
                            recentFiles.recentFiles.clear();
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit"))
                    {
                        mApp->doExit();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Emulation"))
                {
                    if (ImGui::MenuItem("Reset"))
                    {
                        mApp->reset();
                    }
                    if (ImGui::MenuItem("Power Off"))
                    {
                        mApp->powerOff();
                    }
                    if (ImGui::MenuItem("Pause", nullptr, mApp->isPaused()))
                    {
                        mApp->pause();
                    }
                    ImGui::Separator();
                    if (ImGui::BeginMenu("Model"))
                    {
                        if (ImGui::BeginMenu("GB game"))
                        {
                            if (ImGui::MenuItem("DMG (Game Boy)", nullptr, mConfig->dmgGameEmulatedConsole == Gameboy::SystemType::DMG))
                            {
                                mConfig->dmgGameEmulatedConsole = Gameboy::SystemType::DMG;
                            }
                            if (ImGui::MenuItem("CGB (Game Boy Color)", nullptr, mConfig->dmgGameEmulatedConsole == Gameboy::SystemType::CGB))
                            {
                                mConfig->dmgGameEmulatedConsole = Gameboy::SystemType::CGB;
                            }
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("GBC game"))
                        {
                            if (ImGui::MenuItem("DMG (Game Boy)", nullptr, mConfig->cgbGameEmulatedConsole == Gameboy::SystemType::DMG))
                            {
                                mConfig->cgbGameEmulatedConsole = Gameboy::SystemType::DMG;
                            }
                            if (ImGui::MenuItem("CGB (Game Boy Color)", nullptr, mConfig->cgbGameEmulatedConsole == Gameboy::SystemType::CGB))
                            {
                                mConfig->cgbGameEmulatedConsole = Gameboy::SystemType::CGB;
                            }
                            ImGui::EndMenu();
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Video"))
                {
                    if (ImGui::MenuItem("Fullscreen", nullptr, mConfig->fullscreen))
                    {
                        mConfig->fullscreen = !mConfig->fullscreen;
                        mApp->setVideoParameters();
                    }
                    if (ImGui::MenuItem("Exclusive fullscreen", nullptr, mConfig->exclusiveFullscreen))
                    {
                        mConfig->exclusiveFullscreen = !mConfig->exclusiveFullscreen;
                        mApp->setVideoParameters();
                    }
                    if (ImGui::MenuItem("Integer scaling", nullptr, mConfig->integerScaling))
                    {
                        mConfig->integerScaling = !mConfig->integerScaling;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Audio"))
                {
                    if (ImGui::BeginMenu("Sample rate (Hz)"))
                    {
                        if (ImGui::MenuItem("8000", nullptr, mConfig->audioSampleRate == 8000))
                        {
                            setAudioSampleRate(8000);
                        }
                        if (ImGui::MenuItem("11025", nullptr, mConfig->audioSampleRate == 11025))
                        {
                            setAudioSampleRate(11025);
                        }
                        if (ImGui::MenuItem("22050", nullptr, mConfig->audioSampleRate == 22050))
                        {
                            setAudioSampleRate(22050);
                        }
                        if (ImGui::MenuItem("44100", nullptr, mConfig->audioSampleRate == 44100))
                        {
                            setAudioSampleRate(44100);
                        }
                        if (ImGui::MenuItem("48000", nullptr, mConfig->audioSampleRate == 48000))
                        {
                            setAudioSampleRate(48000);
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Quality"))
                    {
                        if (ImGui::MenuItem("Low", nullptr, mConfig->audioQuality == Config::AudioQuality::Low))
                        {
                            mConfig->audioQuality = Config::AudioQuality::Low;
                        }
                        if (ImGui::MenuItem("High", nullptr, mConfig->audioQuality == Config::AudioQuality::High))
                        {
                            mConfig->audioQuality = Config::AudioQuality::High;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                std::string fpsString = std::format("{:.0f} FPS", io.Framerate);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                     std::max(0.0F, ImGui::GetContentRegionAvail().x -
                                                        ImGui::CalcTextSize(fpsString.c_str()).x));
                ImGui::Text("%s", fpsString.c_str());
                ImGui::EndMainMenuBar();
            }
        }
        else
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }
    }

    ImGui::Render();

    // Clear screen
    SDL_SetRenderDrawColor(mRendererSDL, 0, 0, 0, 0xFF);
    SDL_RenderClear(mRendererSDL);
    // Update renderer
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), mRendererSDL);
    SDL_RenderPresent(mRendererSDL);
}

void GUI::loadRomFile(std::string &openFilenameString)
{
    mApp->loadRomFile(openFilenameString);
    mApp->recentFiles.lastOpenLocation =
        std::filesystem::path(openFilenameString).remove_filename().string();
    mApp->recentFiles.push(openFilenameString);
}

void GUI::setAudioSampleRate(int audioSampleRate)
{
    mConfig->audioSampleRate = audioSampleRate;
    mApp->setupAudio();
}

void GUI::destroy()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
