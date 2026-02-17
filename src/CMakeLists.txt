cmake_minimum_required(VERSION 3.7)

project(ChimpGB)

set(SOURCE_FILES
    core/Gameboy.cpp
    core/Cartridge.cpp
    core/RomHeader.cpp
    core/CPU.cpp
    core/CPUDebug.cpp
    core/PPU.cpp
    core/APU.cpp
    core/MBC/NoMBC.cpp
    core/MBC/MBC1.cpp
    core/MBC/MBC3.cpp
    core/MBC/MBC5.cpp
    frontend/ChimpGB.cpp
    frontend/ChimpGBApp.cpp
    frontend/Platform.cpp
    frontend/Config.cpp
    frontend/GUI.cpp
    frontend/RecentFiles.cpp
)

set (IMGUI_SOURCE_FILES
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui.cpp
    lib/imgui/backends/imgui_impl_sdl2.cpp
    lib/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

set (MISC_SOURCE_FILES
    lib/tinyfiledialogs/tinyfiledialogs.cpp
)

add_executable(ChimpGB ${SOURCE_FILES} ${IMGUI_SOURCE_FILES} ${MISC_SOURCE_FILES})

include_directories(core lib lib/imgui)
if(EMSCRIPTEN)
    include_directories(${SDL2_INCLUDE_DIRS})
else()
    include_directories(${SDL2_INCLUDE_DIR})
endif()

if (MINGW)
    target_link_libraries(ChimpGB mingw32)
endif()
target_link_libraries(ChimpGB ${SDL2_LIBRARIES})
if (WIN32)
    target_link_libraries(ChimpGB shlwapi)
endif()
if (EMSCRIPTEN)
    set_target_properties(ChimpGB PROPERTIES LINK_FLAGS
        "-sASYNCIFY=1 -sEXPORTED_RUNTIME_METHODS=ccall -lidbfs.js")
endif()
