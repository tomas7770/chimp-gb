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
    MBC/MBC5.cpp
    Config.cpp
    GUI.cpp
)

set (IMGUI_SOURCE_FILES
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_sdlrenderer2.cpp
)

set (MISC_SOURCE_FILES
    tinyfiledialogs/tinyfiledialogs.cpp
)

add_executable(ChimpGB ${SOURCE_FILES} ${IMGUI_SOURCE_FILES} ${MISC_SOURCE_FILES})

include_directories(${SDL2_INCLUDE_DIR} imgui)
if (MINGW)
    target_link_libraries(ChimpGB mingw32)
endif()
target_link_libraries(ChimpGB ${SDL2_LIBRARIES})
if (WIN32)
    target_link_libraries(ChimpGB shlwapi)
endif()
