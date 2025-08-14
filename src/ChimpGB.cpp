#include "ChimpGBApp.h"
#include "Cartridge.h"

#include <iostream>
#include <fstream>

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ChimpGB <rom file> [-debug]" << std::endl;
        return 0;
    }

    std::string filename(args[1]);
    std::ifstream dataStream(filename, std::ios::binary | std::ios::ate);
    auto size = dataStream.tellg();
    dataStream.seekg(0);

    bool debug = false;
    if (argc >= 3 && std::string(args[2]) == "-debug")
    {
        debug = true;
    }

    ChimpGBApp *app = new ChimpGBApp(Cartridge(dataStream, size), debug);
    app->mainLoop();

    return 0;
}
