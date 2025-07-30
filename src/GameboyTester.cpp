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

        std::cout << testNumber << " " << test["name"] << " ";

        bool fail = false;

        for (auto &cyclesValue : test["cycles"])
        {
            gameboy.doMCycle();
            if (cyclesValue.is_null())
            {
                if (gameboy.cycleInfo.accessType != Gameboy::AccessType::NONE)
                {
                    std::cout << "FAIL cycles null " << gameboy.cycleInfo.address << " " << gameboy.cycleInfo.value << " " << gameboy.cycleInfo.accessType;
                    std::cout << "\n";
                    fail = true;
                    break;
                }
                else
                {
                    continue;
                }
            }
            int address = cyclesValue[0];
            if (gameboy.cycleInfo.address != address)
            {
                std::cout << "FAIL cycles address " << address << " " << gameboy.cycleInfo.address;
                std::cout << "\n";
                fail = true;
                break;
            }
            int value = cyclesValue[1];
            if (gameboy.cycleInfo.value != value)
            {
                std::cout << "FAIL cycles value " << value << " " << gameboy.cycleInfo.value;
                std::cout << "\n";
                fail = true;
                break;
            }
            std::string accessType = cyclesValue[2];
            if ((accessType == "read" && gameboy.cycleInfo.accessType != Gameboy::AccessType::READ)
            || (accessType == "write" && gameboy.cycleInfo.accessType != Gameboy::AccessType::WRITE))
            {
                std::cout << "FAIL cycles accessType " << accessType << " " << gameboy.cycleInfo.accessType;
                std::cout << "\n";
                fail = true;
                break;
            }
        }

        if (fail)
        {
            continue;
        }

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
            for (auto &ramValue : test["final"]["ram"])
            {
                int address = ramValue[0];
                if (gameboy.debugRam[address] != ramValue[1])
                {
                    std::cout << "FAIL ram " << ramValue[0] << " " << ramValue[1] << " " << int(gameboy.debugRam[address]);
                    std::cout << "\n";
                    fail = true;
                    break;
                }
            }
            if (fail)
            {
                continue;
            }

            std::cout << "PASS";
        }
        std::cout << "\n";
    }
}
