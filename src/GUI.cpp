#include "GUI.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

#include "Platform.h"
#include "ChimpGBApp.h"

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

    {
        if (mApp->isPoweredOn())
        {
            ImGuiIO &io = ImGui::GetIO();
            float windowRatio = io.DisplaySize.x / io.DisplaySize.y;
            ImVec2 viewportSize;
            if (mConfig->integerScaling)
            {
                int scaleFactor = windowRatio >= 1.0F ? int(io.DisplaySize.y / LCD::SCREEN_H) : int(io.DisplaySize.x / LCD::SCREEN_W);
                viewportSize = ImVec2(float(scaleFactor * LCD::SCREEN_W), float(scaleFactor * LCD::SCREEN_H));
            }
            else
            {
                if (windowRatio >= 1.0F)
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
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
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

void GUI::destroy()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
