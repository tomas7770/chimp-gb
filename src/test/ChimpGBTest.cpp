#include <iostream>
#include <fstream>
#include "Gameboy.h"

void printSerialByte(void *userdata, uint8_t serialByte)
{
    std::cout << serialByte;
}

int main(int argc, char *args[])
{
    // Read args
    std::string filepath;
    int runCycles;
    SystemType systemType;
    bool agb = false;

    if (argc < 4)
    {
        std::cout << "Please provide a ROM file path, number of cycles to run, and system type." << std::endl;
        return -1;
    }

    filepath = args[1];
    try
    {
        runCycles = std::stoi(args[2]);
        systemType = static_cast<SystemType>(std::stoi(args[3]));
    }
    catch (std::exception err)
    {
        std::cout << "Number of cycles and system type must be integers." << std::endl;
        return -1;
    }

    for (int i = 4; i < argc; i++)
    {
        if (std::string(args[i]) == "-agb")
        {
            agb = true;
        }
    }

    // Load ROM
    std::ifstream dataStream(filepath, std::ios::binary | std::ios::ate);
    if (!dataStream.good())
    {
        std::cout << "Error loading requested ROM. Perhaps this file doesn't exist?" << std::endl;
        return -1;
    }
    auto size = dataStream.tellg();
    dataStream.seekg(0);

    Cartridge cart = Cartridge(dataStream, size);
    Gameboy *gameboy = new Gameboy(cart, false, systemType);
    gameboy->setSerialCallback(printSerialByte, nullptr);
    gameboy->simulateBootRom(agb);
    for (int i = 0; i < runCycles; i++)
    {
        gameboy->doCycle(false);
    }

    delete gameboy;

    return 0;
}
