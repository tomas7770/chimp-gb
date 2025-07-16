#include "Cartridge.h"
#include "Gameboy.h"
#include "GameboyTester.h"

#include <iostream>
#include <fstream>

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ChimpGB <test file>" << std::endl;
        return 0;
    }

    std::string filename(args[1]);
    std::ifstream testStream(filename, std::ios::binary | std::ios::ate);
    auto size = testStream.tellg();
    testStream.seekg(0);

    auto gameboy = new Gameboy();
    runTestSuite(testStream, *gameboy);
    delete gameboy;

    return 0;
}
