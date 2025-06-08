cmake_minimum_required(VERSION 3.7)

project(ChimpGB)

set(SOURCE_FILES
    ChimpGB.cpp
    Gameboy.cpp
    Cartridge.cpp
    RomHeader.cpp
    CPU.cpp
)

add_executable(ChimpGB ${SOURCE_FILES})

include_directories(${SDL2_INCLUDE_DIR})
if (MINGW)
    target_link_libraries(ChimpGB mingw32)
endif()
target_link_libraries(ChimpGB ${SDL2_LIBRARIES})
if (WIN32)
    target_link_libraries(ChimpGB shlwapi)
endif()
