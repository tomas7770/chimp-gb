# ChimpGB - Game Boy Emulator
## Note: This project is a WIP.

ChimpGB is an open source, cross platform Game Boy (DMG) emulator written in C++.

<picture>
    <img src="images/screenshot.png" alt="ChimpGB running Tetris">
</picture>

## Development status

- Passes 100% of the [Gameboy CPU (Sharp LR35902) Tests](https://github.com/SingleStepTests/GameboyCPUTests)

- Passes 100% of [Blargg's](https://github.com/retrio/gb-test-roms/tree/master) cpu\_instrs, instr\_timing, and mem\_timing tests

- Passes [dmg-acid2](https://github.com/mattcurrie/dmg-acid2) PPU test

- Most GB-compatible games should run

- Partial GBC support

Detailed status:

- **CPU:** fully implemented, except for **STOP** opcode

- **PPU/LCD:** mostly implemented

- **APU:** mostly implemented

- **Implemented MBCs:**

    - MBC1 with ROM <= 512 KiB, RAM and battery

    - MBC3 with RAM, battery, and RTC

    - MBC5 with RAM and battery

## Compiling

ChimpGB requires the following tools and libraries to build:

- CMake 3.7 or newer

- C++20 compiler (such as g++)

- Make/Ninja or equivalent

- SDL2

Run the following commands:

```
cmake -S. -Bbuild
cmake --build build
```

## Usage

Run ChimpGB from the command line:

```
ChimpGB <rom file> [-debug]
```

`<rom file>` is the path to a valid Game Boy ROM. `-debug` is an optional parameter that prints executed CPU opcodes to stdout.

## Default controls

- **D-Pad:** Arrow keys

- **A:** Z

- **B:** X

- **Start:** Enter

- **Select:** Right shift

- **Fast Forward:** Tab

- **Fullscreen:** F11

Controls can be changed in the configuration file.

## Saves location

- **Windows:** `C:\Users\username\Saved Games\ChimpGB`

- **Linux:** `$XDG_DATA_HOME/ChimpGB`

    - If `$XDG_DATA_HOME` is not set, it defaults to `~/.local/share`.

Note that not all games save data, in which case a save file won't be present.

## Config location

- **Windows:** `C:\Users\username\AppData\Local\ChimpGB\ChimpGB.ini`

- **Linux:** `$XDG_CONFIG_HOME/ChimpGB/ChimpGB.ini`

    - If `$XDG_CONFIG_HOME` is not set, it defaults to `~/.config`.

## References

This is a non-exhaustive list of references used in the development of this emulator. It mostly consists of technical documentation about Game Boy hardware, and useful tests that ensure the correctness of the emulator.

- [Game Boy Pan Docs](https://gbdev.io/pandocs/)

- [Game Boy: Complete Technical Reference](https://gekkio.fi/files/gb-docs/gbctr.pdf)

- [RGBDS CPU opcode reference](https://rgbds.gbdev.io/docs/v0.9.3/gbz80.7)

- [Game Boy CPU internals](https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595)

- [Gameboy sound hardware](https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware)

- [MBC3 RTC save format](https://bgb.bircd.org/rtcsave.html)

- [Gameboy CPU (Sharp LR35902) Tests](https://github.com/SingleStepTests/GameboyCPUTests)

- [Blargg's Gameboy hardware test ROMs](https://github.com/retrio/gb-test-roms)

- [Gameboy Doctor](https://github.com/robert/gameboy-doctor)

- [dmg-acid2](https://github.com/mattcurrie/dmg-acid2)

- [MBC3 RTC test ROM](https://github.com/aaaaaa123456789/rtc3test)

## Credits

ChimpGB was developed by [tomas7770](https://github.com/tomas7770).

It contains the following third-party libraries:

- [A fork of mINI](https://github.com/IruzzArcana/mINI) with support for stringstream.
