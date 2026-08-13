#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// HOST SIDE DISPLAY SIM
// doesnt emulate GC9107 spi protocol yet, renders logical framebuffer as three physical 128x128 panels
int display_init(void);
void display_shutdown(void);


// return false when user want to quit.
bool display_process_events(void);

// draw packed 1 bpp vram
void display_present(
    const uint8_t *vram,
    size_t vram_size
);

#endif