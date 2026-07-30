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

- AMD Am486DX4 or Am5x86 processor (option will be provided to also be able to use Intel via jumper/switches)
- ecp5-85k fpga chipset
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
- optionally half life 2 support (for the guy whose spreadsheets ran so well)

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
## References

this project studies and builds upon publicly documented work including:

- [M8SBC-486](https://github.com/maniekx86/M8SBC-486/tree/main)
- [FoxCutter 486Dev](https://github.com/FoxCutter/486Dev/)
- [ao486_MiSTer](https://github.com/MiSTer-devel/ao486_MiSTer)
- AMD Am486 development-platform documentation (will be provided in the docs)
- Intel and AMD 486 hardware manuals (same as above)

Reused material will be clearly attributed and used according to its licence (when i get around to it, I WILL).

## Current status

Early architecture and research stage.

## Licence

GNU GPL v3
