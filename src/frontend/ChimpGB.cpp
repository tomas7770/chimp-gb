#include "ChimpGBApp.h"
#include "Cartridge.h"

#include <iostream>
#include <fstream>
#include <filesystem>

static ChimpGBApp *app;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
extern "C"
{
    void EMSCRIPTEN_KEEPALIVE loadDownloadedRom(const char *filepathChars)
    {
        if (app != nullptr)
        {
            std::string filepath = filepathChars;
            app->loadRomFile(filepath);
            app->recentFiles.push(filepath);
        }
    }

    void EMSCRIPTEN_KEEPALIVE onWebPageHidden()
    {
        app->saveConfig();
    }
}
#endif

int main(int argc, char *args[])
{
    std::string filepath = "";
    bool debug = false;
    for (int i = 1; i < argc; i++)
    {
        if (std::string(args[i]) == "-debug")
        {
            debug = true;
        }
        else
        {
            filepath = args[i];
        }
    }

    app = new ChimpGBApp(filepath, debug);
    app->startMainLoop();

    return 0;
}
