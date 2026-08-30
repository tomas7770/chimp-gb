import sys, subprocess

exe = None

def setup():
    assert(len(sys.argv) >= 2)
    global exe
    exe = sys.argv[1]

def runRom(filepath, runCycles, systemType, agb = False):
    result = subprocess.run([exe, filepath, str(runCycles), str(systemType), "-agb" if agb else ""], capture_output = True)
    return result.stdout

def test(filepath, runCycles, systemType, expected, agb = False):
    return {
        "filepath": filepath,
        "runCycles": runCycles,
        "systemType": systemType,
        "expected": expected,
        "agb": agb,
    }

tests = (
    test("./gb-test-roms/cpu_instrs/cpu_instrs.gb", 63000000, 1, b'cpu_instrs\n\n01:ok  02:ok  03:ok  04:ok  05:ok  06:ok  07:ok  08:ok  09:ok  10:ok  11:ok  \n\nPassed all tests\n'),
    test("./gb-test-roms/instr_timing/instr_timing.gb", 1400000, 1, b'instr_timing\n\n\nPassed\n'),
    test("./gb-test-roms/mem_timing/mem_timing.gb", 3300000, 1, b'mem_timing\n\n01:ok  02:ok  03:ok  \n\nPassed all tests\n'),

    test("./mts-20240926-1737-443f6e1/emulator-only/mbc1/rom_512kb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc1/rom_1Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc1/rom_2Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc1/rom_4Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),

    test("./mts-20240926-1737-443f6e1/emulator-only/mbc5/rom_512kb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc5/rom_1Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc5/rom_2Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc5/rom_4Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc5/rom_8Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc5/rom_16Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc5/rom_32Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/emulator-only/mbc5/rom_64Mb.gb", 500000, 0, b'\x03\x05\x08\r\x15"'),

    test("./mts-20240926-1737-443f6e1/acceptance/boot_regs-dmgABC.gb", 400000, 0, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/acceptance/boot_regs-dmgABC.gb", 400000, 1, b'BBBBBB'),
    test("./mts-20240926-1737-443f6e1/acceptance/boot_regs-dmgABC.gb", 400000, 1, b'BBBBBB', agb = True),

    test("./mts-20240926-1737-443f6e1/misc/boot_regs-cgb.gb", 400000, 0, b'BBBBBB'),
    test("./mts-20240926-1737-443f6e1/misc/boot_regs-cgb.gb", 400000, 1, b'\x03\x05\x08\r\x15"'),
    test("./mts-20240926-1737-443f6e1/misc/boot_regs-cgb.gb", 400000, 1, b'BBBBBB', agb = True),

    test("./mts-20240926-1737-443f6e1/misc/boot_regs-A.gb", 400000, 0, b'BBBBBB'),
    test("./mts-20240926-1737-443f6e1/misc/boot_regs-A.gb", 400000, 1, b'BBBBBB'),
    test("./mts-20240926-1737-443f6e1/misc/boot_regs-A.gb", 400000, 1, b'\x03\x05\x08\r\x15"', agb = True),
)
total = len(tests)

def systemTypeStr(systemType, agb):
    if agb:
        return "AGB"
    elif systemType == 0:
        return "DMG"
    elif systemType == 1:
        return "CGB"
    else:
        return systemType

def runTests():
    passed = 0
    for t in tests:
        out = runRom(t["filepath"], t["runCycles"], t["systemType"], agb = t["agb"])
        if out == t["expected"]:
            print(f"Test \"{t["filepath"]}\" {systemTypeStr(t["systemType"], t["agb"])}: Pass")
            passed += 1
        else:
            print(f"Test \"{t["filepath"]}\" {systemTypeStr(t["systemType"], t["agb"])}: Fail")
            print(f"Expected: {t["expected"]}")
            print(f"Output: {out}\n")
    print(f"Passed: {passed}/{total} ({round(100*passed/total)}%)")
    assert(passed == total)

if __name__ == '__main__':
    setup()
    runTests()
