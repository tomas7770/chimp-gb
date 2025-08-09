#include "Cartridge.h"
#include "Gameboy.h"

#include <iostream>
#include <fstream>
#include "SDL.h"

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ChimpGB <rom file>" << std::endl;
        return 0;
    }

    std::string filename(args[1]);
    std::ifstream dataStream(filename, std::ios::binary | std::ios::ate);
    auto size = dataStream.tellg();
    dataStream.seekg(0);

    auto gameboy = new Gameboy(Cartridge(dataStream, size));

    uint64_t frameTimestamp = SDL_GetTicks64();
    while (1)
    {
        // TODO: make it possible to stop
        uint64_t deltaTime = SDL_GetTicks64() - frameTimestamp;
        frameTimestamp = SDL_GetTicks64();

        gameboy->tick(1e6 * deltaTime);
    }

    delete gameboy;

    return 0;
}
