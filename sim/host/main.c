#include "display.h"
#include "cpu.h"
#include "machine.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

//init ram test
static int test_ram(void)
{
    uint32_t address;

    // pattern 1
    for (address = RAM_BASE; address < RAM_BASE + RAM_SIZE; address++) {
        machine_mem_write8(address, (uint8_t)(address & 0xFF));
    }
    for (address = RAM_BASE; address < RAM_BASE + RAM_SIZE; address++) {
        uint8_t expected;
        uint8_t actual;
        expected = (uint8_t)(address & 0xFF);
        actual = machine_mem_read8(address);
        if (actual != expected) {
            fprintf(stderr, "RAM FAIL at %05X: expected %02X got %02X\n", (unsigned)address, expected, actual);
            return -1;
        }
    }

    // pattern 2: AA / 55
    for (address = RAM_BASE; address < RAM_BASE + RAM_SIZE; address++) {
        machine_mem_write8(address, (address & 1u) ? 0x55 : 0xAA);
    }
    for (address = RAM_BASE; address < RAM_BASE + RAM_SIZE; address++) {
        uint8_t expected;
        uint8_t actual;
        expected = (address & 1u) ? 0x55 : 0xAA;
        actual = machine_mem_read8(address);
        if (actual != expected) {
            fprintf(stderr, "RAM FAIL at %05X: expected %02X got %02X\n", (unsigned)address, expected, actual);
            return -1;
        }
    }
    printf("RAM test: OK (%u bytes)\n", (unsigned)RAM_SIZE);
    return 0;
}

// temp graphics test 
static void draw_test_screen(void)
{
    int x;
    int y;
    machine_video_clear(false);

    // outer border
    for (x = 0; x < VIDEO_WIDTH; x++) {
        machine_video_set_pixel(x, 0, true);
        machine_video_set_pixel(x, VIDEO_HEIGHT - 1, true);
    }
    for (y = 0; y < VIDEO_HEIGHT; y++) {
        machine_video_set_pixel(0, y, true);
        machine_video_set_pixel(VIDEO_WIDTH - 1, y, true);
    }

    // show boundary between the three logical panels
    for (y = 0; y < VIDEO_HEIGHT; y++) {
        machine_video_set_pixel(127, y, true);
        machine_video_set_pixel(128, y, true);
        machine_video_set_pixel(255, y, true);
        machine_video_set_pixel(256, y, true);
    }

    // repeating diagonal across all three screen
    for (x = 0; x < VIDEO_WIDTH; x++) {
        y = x % VIDEO_HEIGHT;
        machine_video_set_pixel(x, y, true);
        machine_video_set_pixel(x, (VIDEO_HEIGHT - 1) - y, true);
    }
}

// debug output helper
static void debug_print(const char *text)
{
    while (*text != '\0') {
        machine_io_write8( DEBUG_PORT, (uint8_t)*text);
        text++;
    }
}

// entry point (heh roblos reference)
int main(int argc, char **argv)
{
    machine_init();
    printf("simulater, will sim for u later\n");
    printf("-------------------------------\n");
    printf("RAM : %u KiB @ %05Xh\n", (unsigned)(RAM_SIZE / 1024), (unsigned)RAM_BASE);
    printf("VRAM: %u bytes @ %05Xh (%dx%d 1bpp)\n", (unsigned)VRAM_SIZE, (unsigned)VRAM_BASE, VIDEO_WIDTH, VIDEO_HEIGHT);
    printf("ROM : %u KiB @ %05Xh\n", (unsigned)(ROM_SIZE / 1024), (unsigned)ROM_BASE);

    // optional:
    //     ./sim bios.bin
    if (argc >= 2) {
        if (machine_load_rom(argv[1]) != 0) {
            return 1;
        }
    } else {
        printf(
            "you stupid you supplied no bios, continuing with blank FF ROM.\n"
        );
    }
    if (cpu_init() != 0) {
    return 1;
    }
    if (test_ram() != 0) {
        return 1;
    }
    debug_print("DEBUG PORT ONLINE\r\n");
    draw_test_screen();
    if (display_init() != 0) {
        return 1;
    }
    printf("\nControls:\n");
    printf("  ESC  quit\n");
    printf("  B    toggle LCD bezels\n");
    printf("  -    reduce bezel size\n");
    printf("  =    increase bezel size\n");
    while (display_process_events()) {
        cpu_run_slice(20000);
        display_present(machine_vram_const(), VRAM_SIZE);

        // ~60 Hz host refresh
        // THIS HAS NOTHING TO DO WITH EVENTUAL LCD/8088 TIMING FUCK OFF
        SDL_Delay(16);
    }
    cpu_shutdown();
    display_shutdown();
    return 0;
}