#pragma once

struct Joypad
{
    bool keys[8] = {false}; // Order of joypad keys: Right, Left, Up, Down, A, B, Select, Start
    bool selectButtons = false;
    bool selectDPad = false;

    static constexpr int SEL_BUTTONS_BITMASK = (1 << 5);
    static constexpr int SEL_DPAD_BITMASK = (1 << 4);
    static constexpr int START_DOWN_BITMASK = (1 << 3);
    static constexpr int SELECT_UP_BITMASK = (1 << 2);
    static constexpr int B_LEFT_BITMASK = (1 << 1);
    static constexpr int A_RIGHT_BITMASK = (1 << 0);

    static constexpr int BUTTONS_INDEX = 4;
};
