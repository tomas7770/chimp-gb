#include "ChimpGBApp.h"
#include "Cartridge.h"

#include <iostream>
#include <fstream>
#include <filesystem>

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

    ChimpGBApp *app = new ChimpGBApp(filepath, debug);
    app->startMainLoop();

    return 0;
}
