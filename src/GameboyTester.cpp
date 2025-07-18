#include "GameboyTester.h"

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void runTestSuite(std::istream &testStream, Gameboy &gameboy)
{
    int testNumber = 0;
    json testSuite = json::parse(testStream);
    for (auto &test : testSuite)
    {
        testNumber++;
        uint16_t initialPC = test["initial"]["pc"];
        uint8_t initialOpcode;
        for (auto &ramValue : test["initial"]["ram"])
        {
            int address = ramValue[0];
            gameboy.debugRam[address] = ramValue[1];
            if (address == initialPC - 1)
                initialOpcode = ramValue[1];
        }
        gameboy.setCPUState({
            test["initial"]["a"],
            test["initial"]["f"],
            test["initial"]["b"],
            test["initial"]["c"],
            test["initial"]["d"],
            test["initial"]["e"],
            test["initial"]["h"],
            test["initial"]["l"],
            test["initial"]["sp"],
            test["initial"]["pc"],
            initialOpcode,
        });
        gameboy.doMCycle();

        std::cout << testNumber << " " << test["name"] << " ";

        auto cpuState = gameboy.getCPUState();
        if (cpuState.regA != test["final"]["a"])
        {
            std::cout << "FAIL a " << test["final"]["a"] << " " << int(cpuState.regA);
        }
        else if (cpuState.regB != test["final"]["b"])
        {
            std::cout << "FAIL b " << test["final"]["b"] << " " << int(cpuState.regB);
        }
        else if (cpuState.regC != test["final"]["c"])
        {
            std::cout << "FAIL c " << test["final"]["c"] << " " << int(cpuState.regC);
        }
        else if (cpuState.regD != test["final"]["d"])
        {
            std::cout << "FAIL d " << test["final"]["d"] << " " << int(cpuState.regD);
        }
        else if (cpuState.regE != test["final"]["e"])
        {
            std::cout << "FAIL e " << test["final"]["e"] << " " << int(cpuState.regE);
        }
        else if (cpuState.regF != test["final"]["f"])
        {
            std::cout << "FAIL f " << test["final"]["f"] << " " << int(cpuState.regF);
        }
        else if (cpuState.regH != test["final"]["h"])
        {
            std::cout << "FAIL h " << test["final"]["h"] << " " << int(cpuState.regH);
        }
        else if (cpuState.regL != test["final"]["l"])
        {
            std::cout << "FAIL l " << test["final"]["l"] << " " << int(cpuState.regL);
        }
        else if (cpuState.PC != test["final"]["pc"])
        {
            std::cout << "FAIL pc " << test["final"]["pc"] << " " << int(cpuState.PC);
        }
        else if (cpuState.SP != test["final"]["sp"])
        {
            std::cout << "FAIL sp " << test["final"]["sp"] << " " << int(cpuState.SP);
        }
        else
        {
            bool fail = false;
            for (auto &ramValue : test["final"]["ram"])
            {
                int address = ramValue[0];
                if (gameboy.debugRam[address] != ramValue[1])
                {
                    std::cout << "FAIL ram " << ramValue[0] << " " << ramValue[1] << " " << int(gameboy.debugRam[address]);
                    fail = true;
                    break;
                }
            }
            if (fail)
            {
                continue;
            }

            for (auto &cyclesValue : test["cycles"])
            {
                // TODO
            }
            if (fail)
            {
                continue;
            }

            std::cout << "CYCLES";
        }
        std::cout << "\n";
    }
}
