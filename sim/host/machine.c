#include "machine.h"
#include <stdio.h>
#include <string.h>

static uint8_t g_ram[RAM_SIZE];
static uint8_t g_vram[VRAM_SIZE];
static uint8_t g_rom[ROM_SIZE];

//lifecycle
void machine_init(void)
{
    memset(g_ram, 0x00, sizeof(g_ram));
    memset(g_vram, 0x00, sizeof(g_vram));
    // empty/unprogram flash/rom normally read high and make missing bios content obvious
    memset(g_rom, 0xFF, sizeof(g_rom));
}

void machine_reset(void)
{
    // cpu reset no wipe ram or rom. cpu reg reset happen in 8088 emu, not here
    // so for now there nothing else to reset
}

// rom load
int machine_load_rom(const char *path)
{
    FILE *file;
    long size;
    size_t bytes_read;
    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "machine: could not open ROM: %s\n", path);
        return -1;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "machine: could not seek ROM\n");
        fclose(file);
        return -1;
    }
    size = ftell(file);
    if (size < 0) {
        fprintf(stderr, "machine: could not determine ROM size\n");
        fclose(file);
        return -1;
    }
    if ((size_t)size != ROM_SIZE) {
        fprintf(stderr, "machine: BIOS must be exactly %u bytes; got %ld\n", (unsigned)ROM_SIZE, size);
        fclose(file);
        return -1;      // L bozo
    }
    rewind(file);
    bytes_read = fread(g_rom, 1, ROM_SIZE, file);
    fclose(file);
    if (bytes_read != ROM_SIZE) {
        fprintf(stderr, "machine: failed while reading BIOS\n");
        return -1;
    }
    printf("machine: loaded %u-byte BIOS from %s\n", (unsigned)ROM_SIZE, path);
    return 0;
}

//membus
uint8_t machine_mem_read8(uint32_t address)
{
    address &= ADDRESS_MASK;
    // main ram
    if (address >= RAM_BASE && address < RAM_BASE + RAM_SIZE) {
        return g_ram[address - RAM_BASE];
    }
    // vidram
    if (address >= VRAM_BASE && address < VRAM_BASE + VRAM_SIZE) {
        return g_vram[address - VRAM_BASE];
    }
    // bios rom
    if (address >= ROM_BASE && address < ROM_BASE + ROM_SIZE) {
        return g_rom[address - ROM_BASE];
    }
    // unmapped, left for now because cba
    return 0xFF;
}

void machine_mem_write8(uint32_t address, uint8_t value)
{
    address &= ADDRESS_MASK;
    // main ram
    if (address >= RAM_BASE && address < RAM_BASE + RAM_SIZE) {
        g_ram[address - RAM_BASE] = value;
        return;
    }
    // vidram
    if (address >= VRAM_BASE &&address < VRAM_BASE + VRAM_SIZE) {
        g_vram[address - VRAM_BASE] = value;
        return;
    }
    // fuck rom and unmapped writes for now, cba
}

//io bus
uint8_t machine_io_read8(uint16_t port)
{
    switch (port) {
        default:
            // unimplemented because I CANT BE ARSED ill do it later how many times i gotta say it
            return 0xFF;
    }
}

void machine_io_write8(uint16_t port, uint8_t value)
{
    switch (port) {
        case DEBUG_PORT:
            // bochs style debug output
            // eventually this will do
            //     mov dx, 0E9h
            //     mov al, 'G'
            //     out dx, al
            fputc(value, stdout);
            fflush(stdout);
            break;

        default:
            // for later
            // 300h-303h  atemga328p controller
            // 310h+      video control
            // 388h       opl2
            // type shit
            break;
    }
}

//vram access
uint8_t *machine_vram(void)
{
    return g_vram;
}

const uint8_t *machine_vram_const(void)
{
    return g_vram;
}

//video helpers
void machine_video_clear(bool on)
{
    memset(g_vram, on ? 0xFF : 0x00, sizeof(g_vram));
}

void machine_video_set_pixel(int x, int y, bool on)
{
    size_t byte_index;
    uint8_t mask;
    if (x < 0 ||x >= VIDEO_WIDTH ||y < 0 ||y >= VIDEO_HEIGHT) {
        return;
    }
    byte_index =
        ((size_t)y * VRAM_ROW_BYTES) +
        ((size_t)x >> 3);
    // msb
    // x=0 -> bit 7
    // x=7 -> bit 0
    mask = (uint8_t)(0x80u >> (x & 7));
    if (on) {
        g_vram[byte_index] |= mask;
    } 
    else {
        g_vram[byte_index] &= (uint8_t)~mask;
    }
}

bool machine_video_get_pixel(int x, int y)
{
    size_t byte_index;
    uint8_t mask;
    if (x < 0 || x >= VIDEO_WIDTH || y < 0 || y >= VIDEO_HEIGHT) {
        return false;
    }
    byte_index = ((size_t)y * VRAM_ROW_BYTES) + ((size_t)x >> 3);
    mask = (uint8_t)(0x80u >> (x & 7));
    return (g_vram[byte_index] & mask) != 0;
}