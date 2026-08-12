# The Grip

**The Grip** is an open source 8088 based palmtop computer project inspired by machines such as the HP 200LX.

The goal is to build a small battery powered x86 computer from understandable parts like a real 8088 CPU, SRAM, ROM, simple programmable glue logic, an embedded peripheral controller, removable storage, and a custom wide display system.

The machine is being developed simulation first and hardware is only purchased or fabricated after the corresponding subsystem has been implemented and tested in the pc sim.

## Project pivot

The Grip originally began as a 486/Am5x86 laptop motherboard project using an FPGA chipset and SDRAM which is now preserved in the repository under ```archived```, but the immediate project has moved to an 8088-based architecture, due to funding constraints (I'm broke).

The 8088 design is substantially cheaper to prototype and allows the system to be developed incrementally on breadboards before committing to a custom pcb. The breadboard part is important, because it allows anyone to build this thing on their own at home without needing pcb prototyping, as not everyone has the time to sit down and use eda software.

The long-term idea remains the same: build an open, understandable and portable x86 computer from the hardware upward. 

### *Why?*
Because getting older machines is getting stupidly expensive and decades old hardware comes with aging displays (vinegar syndrome, blown backlight, fading display etc.), capacitors and batteries(very leaky little shits), plastics and other parts that can be difficult or expensive to replace (and again, if soft plastic, will turn into goo). 
eBay listings charge too much for even things that are dead and for parts. The aim is to build something with a similar spirit and capability from replaceable and documented parts without paying expensive prices (and what I personally feel are set ridiculously) for aging hardware. 
It is not an exact replica, but a similar era replication, using modern parts where needed be when old chips cannot be procured easily. 
It is a device born out of a wish to use old hardware, while not paying through the nose for it.

Also the customs in my country sucks. They don't let me import old second hand devices without a funky ass ewaste license.

## Initial target

The first hardware milestone is to:

* execute code from the reset vector on a real 8088
* provide at least 32 KB of working SRAM during initial bring-up
* move to 512 KB SRAM for the complete system
* boot from a 29C256 ROM
* provide serial/debug output
* communicate with an embedded peripheral controller
* operate a simulated and later physical display
* eventually boot DOS

## Planned system

Current provisional architecture:

```text
CPU
    Intel-compatible 8088

Memory
    512 KB SRAM
    32 KB 29C256 boot ROM

Glue logic
    74HCT/74LS logic
    Vicharak Shrike Lite where useful
    Tang Primer 20K for later programmable logic/video work
    Eventually replace programmable glue with era appropriate parts where practical

Peripheral controller
    ATmega328P

Storage
    MicroSD

RTC
    PCF8563

Display
    Three 128×128 SPI LCD panels
    384×128 combined physical resolution

    Optional bezel-corrected virtual mode where the physical gaps between displays are treated as hidden pixels.

Software
    custom BIOS
    DOS
    custom PDA-style graphical wrapper
    native PC sim
```

The exact hardware map is intentionally not frozen yet. The simulator will be used to validate these decisions before the final hardware is designed.

## Display concept

The current display concept uses three small LCD panels side-by-side behind a single dark front window.

```text
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│              │ │              │ │              │
│   128×128    │ │   128×128    │ │   128×128    │
│              │ │              │ │              │
└──────────────┘ └──────────────┘ └──────────────┘

                    384 × 128
```

The PDA interface will be aware of the physical display boundaries and avoid placing important UI elements across them.

Software that benefits from one continuous field of view may instead use a bezel-corrected virtual display where pixels hidden behind the physical gaps remain part of the logical coordinate system.

## Software-first development

A PC-hosted simulator is a main part of the project rather than a temporary debugging tool. (I like to test my stuff before I put it on real hardware, because the wonders of modern hardware let me do so.)

The simulator is intended to model:

```text
8088/8086 compatible CPU
RAM and ROM
I/O address space
three-panel display
keyboard
RTC
SD storage
embedded controller
```

Development will initially use simplified host-side peripheral models.

Once the interfaces are stable, these models can progressively be replaced with more accurate implementations, including execution of the real ATmega328P firmware in an AVR simulator.

The same high-level wrapper/UI code should be able to target both the native simulator and the eventual DOS machine through a small platform abstraction layer.

## Proposed memory map

Early provisional map:

```text
00000h - 7FFFFh    Main RAM

B8000h - B9FFFh    Reserved video region

F8000h - FFFFFh    32 KB system ROM
```

This map may change while the simulator and BIOS are developed.

## Development stages

1. Define architecture, memory map and I/O interfaces.
2. Build the native PC display/peripheral simulator.
3. Build the PDA wrapper UI in the simulator.
4. Integrate an 8088/8086-compatible CPU emulator.
5. Execute the first custom BIOS from the reset vector.
6. Implement simulated keyboard, RTC and SD peripherals.
7. Define and test the ATmega328P communication protocol.
8. Run the real ATmega firmware under simulation.
9. Bring up the physical 8088 breadboard.
10. Add physical storage and keyboard support.
11. Add the three-panel display.
12. Add portable battery power.
13. Move proven subsystems onto a custom motherboard.
14. Build the final palmtop enclosure.

## Repository structure

```text
docs/          Architecture, memory maps, protocols and notes
sim/           Host simulator, CPU integration and device models
firmware/      BIOS and embedded-controller firmware
software/      DOS applications and the PDA wrapper
rtl/           FPGA glue logic and video logic
hardware/      Breadboard schematics, carrier boards and PCBs
mechanical/    Enclosure and mounting designs
bom/           Bills of materials and sourcing information
journal/       Development logs
references/    Reference material and attribution notes
```

Historical 486 design work is retained under the ```archived``` directory.

## Current status

**Simulation and architecture stage.**

Immediate objective:

```text
PC simulator
    ↓
three-panel display model
    ↓
machine memory/I/O model
    ↓
8088 reset-vector execution
    ↓
first custom BIOS
```

## Long-term software goals

The first usable software environment will likely consist of:

```text
BIOS
  ↓
DOS
  ↓
The Grip graphical wrapper
```

The wrapper will provide a PDA-style interface for applications such as a file manager, notes, calendar, terminal and system settings.

More ambitious compatibility work, including Windows 3.x and other 8088 operating environments, can be explored after the base machine is stable.

## Licensing

The intended licensing model remains:

| Component             | License        |
| --------------------- | -------------- |
| HDL and firmware      | GNU GPLv3      |
| Hardware design files | CERN-OHL-S-2.0 |
| Documentation         | CC BY-SA 4.0   |

Third-party components and emulator code retain their respective original licences and notices.
