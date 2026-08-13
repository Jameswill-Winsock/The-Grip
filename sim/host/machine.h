#ifndef MACHINE_H
#define MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ADDRESS_BITS 20u
#define ADDRESS_SPACE (1u << ADDRESS_BITS)
#define ADDRESS_MASK (ADDRESS_SPACE - 1u)

#define PANEL_WIDTH 128
#define PANEL_HEIGHT 128
#define PANEL_COUNT 3
#define DISPLAY_WIDTH (PANEL_WIDTH*PANEL_COUNT)
#define DISPLAY_HEIGHT PANEL_HEIGHT
#define BEZEL_DEAD_WIDTH 8
#define BEZEL_WIDE_WIDTH (DISPLAY_WIDTH + (BEZEL_WIDTH*2))

#define RAM_BASE 0X00000
#define RAM_SIZE 0X08000
// #define RAM_SIZE 0x80000 for when i get 512kb ram later
#define ROM_BASE 0XF8000
#define ROM_SIZE 0x8000u

#define VIDEO_WIDTH (PANEL_WIDTH*PANEL_COUNT)
#define VIDEO_HEIGHT PANEL_HEIGHT

#define VRAM_BASE 0xB8000U
#define VRAM_ROW_BYTES (VIDEO_WIDTH / 8)
#define VRAM_SIZE (VRAM_ROW_BYTES * VIDEO_HEIGHT)

#define DEBUG_PORT 0X00E9u


//machine lifecycle
void machine_init(void);
void machine_reset(void);

// load exact 32 kib bios image
int machine_load_rom(const char *path);

// membus
uint8_t machine_mem_read8(uint32_t address);
void machine_mem_write8(uint32_t address, uint8_t value);

//iobus
uint8_t machine_io_read8(uint16_t port);
void machine_io_write8(uint16_t port, uint8_t value);

//vid help
uint8_t *machine_vram(void);
const uint8_t *machine_vram_const(void);
void machine_video_clear(bool on);
void machine_video_set_pixel(int x, int y, bool on);
bool machine_video_get_pixel(int x, int y);

#endif