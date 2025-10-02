#include "ChimpGBApp.h"
#include "Cartridge.h"

#include <iostream>
#include <fstream>
#include <filesystem>

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ChimpGB <rom file> [-debug]" << std::endl;
        return 0;
    }

    std::string filepath(args[1]);
    std::ifstream dataStream(filepath, std::ios::binary | std::ios::ate);
    if (!dataStream.good())
    {
        std::cout << "Error loading requested ROM. Perhaps this file doesn't exist?" << std::endl;
        return 0;
    }
    auto size = dataStream.tellg();
    dataStream.seekg(0);

    bool debug = false;
    if (argc >= 3 && std::string(args[2]) == "-debug")
    {
        debug = true;
    }

    std::string romFilename = std::filesystem::path(filepath).filename().stem().string();

    ChimpGBApp *app = new ChimpGBApp(Cartridge(dataStream, size),
                                     romFilename, debug);
    app->mainLoop();

    return 0;
}
