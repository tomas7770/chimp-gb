#include "Cartridge.h"
#include "Gameboy.h"

#include <iostream>
#include <fstream>

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
    
    delete gameboy;

    return 0;
}
