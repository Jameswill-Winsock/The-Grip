# The Grip
x86-32 based open source motherboard for the 486 family, to be eventually used in a laptop configuration.

project will begin as a modular prototype using separate cpu, fpga/memory, display, storage, audio, and controller boards. once arch is proven, the modules will be integrated into a final motherboard and laptop enclosure.

## Initial target

first hardware milestone is to:

- power and reset a real Am486/Am5x86 CPU
- execute code from the reset vector
- output post codes and serial debug messages
- initialize and test 64 mb of sdram
- provide a stable platform for later pc compatible peripherals

## Planned system

- AMD Am486DX4 or Am5x86 processor (option will be provided to also be able to use Intel via selectable voltage, clock multiplier, and cache/configuration straps, but I will do that on a later revision)
- ecp5-85f fpga chipset
- 64 mb sdram
- fpga configuration flash
- bios firmware
- vga compatible display controller
- lcd output
- ide compatible SD storage
- keyboard and touchpad/trackball controller
- pc compatible interrupt, timer, and dma logic
- Audio and floppy support
- dos, linux, win 95 & win 98 compatibility
- optionally half life 2 support as a stretch goal (for the guy whose spreadsheets ran so well)

## Dev stages

1. arch and document
2. cpu bus sim
3. cpu breakout board
4. fpga and sdram carrier
5. rom, post, uart, and memory bring-up
6. storage and keyboard support
7. vga and lcd output
8. audio and floppy support
9. integrated motherboard
10. laptop enclosure and portable power

## Repo structure

```text
docs/        arch, memory maps, plans, and technical notes
rtl/         fpga rtl
sim/         testbenches and simulation models
firmware/    bios, embedded controller, and diagnostic firmware
hardware/    schematics, pcb files, and fabrication outputs
mechanical/  enclosure and mounting designs
bom/         bills of materials and sourcing information
journal/     development logs
references/  reference links and attribution notes
```
## Todo step by step
1. block diagram
2. mem arch
3. voltage/power tree
4. memory and i/o Map
5. simulate 486 reset vector read at 0xFFFFFFF0
6. sim ram r/w transactions
7. port 0x80 post
8. waveform diagrams and timing analysis
9. protocol assertion
10. prototype schematics (initial build)
    - cpu breakout board and clock/rst
    - fpga/sdram carrier
    - level translate
    - uart and post debug lights
    - jtag and config
11. BOM & quote
12. Step by step plan
    1. power
    2. fpga northbridge/southbridge
    3. reset vector fetch
    4. post
    5. ram test
    6. i/o on serial mon

## Temporary BOM and what goes where
1. CPU Breakout
    - Am5x86-P75 or Am486-DX4 100 as CPU
    - Socket 3 PGA 168
    - Voltage Regulator and selection (3.3, 3.45, 5V etc)
    - CLK/RST
    - Config jumper
    - Headers for all pins
    - Bus buffer
    - Level translate
    - 4xSN74LVC245A Level shifter (not exact quantity, to be decided later)
    - 8xSN74LVC244A Bus Buffer (not exact quantity, to be decided later)

2. FPGA/SDRAM carrier
    - ECP5-85F
    - 1xAS4C16M32SC
    - W25Q128 SPI Flash
    - 50 Mhz CMOS oscillator
    - JTAG and other debug boards
    - Optional RP2040 to program FPGA at boot and help with programming
    - 1.1V/2.5V/3.3V voltage regulators

3. Backplane carrier
    - CPU breakout connector
    - FPGA carrier connector
    - Power distribution
    - Ground plane
    - Test headers
    - Expansion connectors

4. Other expansions
    - POST Display and UART
    - RTC
    - Keyboard and Embedded Controller (Another RP2040) (later)
    - IDE/SD Adapter (later)
    - Floppy (later)
    - Audio codec/amplifier (later)
    - Power button and fan controller (later)

5. Display
    - Comes later


## References

this project studies and builds upon publicly documented work including:

- [M8SBC-486](https://github.com/maniekx86/M8SBC-486/tree/main)
- [FoxCutter 486Dev](https://github.com/FoxCutter/486Dev/)
- [ao486_MiSTer](https://github.com/MiSTer-devel/ao486_MiSTer)
- AMD Am486 development-platform documentation (will be provided in the docs)
- Intel and AMD 486 hardware manuals (same as above)

Reused or adapted material will retain its original notices and will be documented in a reference and licence ledger before it is integrated.

## Current status

Early architecture and research stage.

## Licence
licensing is currently under review pending an audit of all referenced hdl and hardware sources (reads: i am trying to figure it out, please be patient, thank you). intended licensing model is:

| Component | License | Badge |
| :--- | :--- | :--- |
| **HDL & Firmware** | [GNU GPLv3](LICENSE-GPL) | [![License: GPL v3](https://shields.io)](https://gnu.org) |
| **Hardware Design Files** | [CERN-OHL-S-2.0](LICENSE-CERN) | [![License: CERN-OHL-S-v2](https://shields.io)](https://spdx.org/licenses/CERN-OHL-S-2.0.html) |
| **Documentation** | [CC BY-SA 4.0](LICENSE-CC) | [![CC BY-SA 4.0](https://creativecommons.org)](https://creativecommons.org) |
