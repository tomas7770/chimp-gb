cmake_minimum_required(VERSION 3.7)

project(ChimpGB)

set(SOURCE_FILES
    ChimpGB.cpp
    ChimpGBApp.cpp
    Platform.cpp
    Gameboy.cpp
    Cartridge.cpp
    RomHeader.cpp
    CPU.cpp
    CPUDebug.cpp
    Timer.cpp
    PPU.cpp
    APU.cpp
    MBC/NoMBC.cpp
    MBC/MBC1.cpp
    MBC/MBC3.cpp
)

add_executable(ChimpGB ${SOURCE_FILES})

include_directories(${SDL2_INCLUDE_DIR})
if (MINGW)
    target_link_libraries(ChimpGB mingw32)
endif()
target_link_libraries(ChimpGB ${SDL2_LIBRARIES} nlohmann_json::nlohmann_json)
if (WIN32)
    target_link_libraries(ChimpGB shlwapi)
endif()
