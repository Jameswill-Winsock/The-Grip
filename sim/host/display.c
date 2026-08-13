#include "display.h"
#include "machine.h"
#include <SDL3/SDL.h>
#include <stdio.h>

// one lcd pixel become 3x3 block on the PC.
#define DISPLAY_SCALE       3

// host window spacing only
// represents the actual dead physical area between LCDs and no exist in actual machine vram
#define DISPLAY_MARGIN      24
#define DISPLAY_DEFAULT_GAP 18
#define DISPLAY_MAX_GAP     120

static SDL_Window   *g_window   = NULL;
static SDL_Renderer *g_renderer = NULL;

static int g_gap = DISPLAY_DEFAULT_GAP;
static int g_saved_gap = DISPLAY_DEFAULT_GAP;

// helper bullshit
static int panel_host_width(void)
{
    return PANEL_WIDTH * DISPLAY_SCALE;
}
static int panel_host_height(void)
{
    return PANEL_HEIGHT * DISPLAY_SCALE;
}
static int window_width(void)
{
    return (DISPLAY_MARGIN * 2) + (panel_host_width() * PANEL_COUNT) + (g_gap * (PANEL_COUNT - 1));
}
static int window_height(void)
{
    return (DISPLAY_MARGIN * 2) + panel_host_height();
}
static float panel_host_x(int panel)
{
    return (float)( DISPLAY_MARGIN + panel * (panel_host_width() + g_gap));
}
static bool vram_pixel(
    const uint8_t *vram, int x, int y
)
{
    size_t byte_index;
    uint8_t mask;
    byte_index = ((size_t)y * VRAM_ROW_BYTES) + ((size_t)x >> 3);
    mask = (uint8_t)(0x80u >> (x & 7));
    return (vram[byte_index] & mask) != 0;
}

// lifecycle
int display_init(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "display: SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }
    if (!SDL_CreateWindowAndRenderer("machine sim", window_width(), window_height(), SDL_WINDOW_RESIZABLE, &g_window, &g_renderer)) {
        fprintf(stderr, "display: could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    return 0;
}

void display_shutdown(void)
{
    if (g_renderer != NULL) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }
    if (g_window != NULL) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
    SDL_Quit();
}

// input
bool display_process_events(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            switch (event.key.key) {
                case SDLK_ESCAPE:
                    return false;

                // B 
                // toggle simulated bezels
                case SDLK_B:
                    if (g_gap == 0) {
                        g_gap = g_saved_gap;
                    } else {
                        g_saved_gap = g_gap;
                        g_gap = 0;
                    }
                    break;

                // - 
                // change physical gap.
                case SDLK_MINUS:
                    if (g_gap > 0) {
                        g_gap -= 3;
                        if (g_gap < 0) {
                            g_gap = 0;
                        }
                        g_saved_gap = g_gap;
                    }
                    break;

                case SDLK_EQUALS:
                    if (g_gap < DISPLAY_MAX_GAP) {
                        g_gap += 3;
                        g_saved_gap = g_gap;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    return true;
}

// render
void display_present(const uint8_t *vram, size_t vram_size)
{
    int panel;
    int x;
    int y;
    if (vram == NULL || vram_size < VRAM_SIZE) {
        return;
    }

    // dark acrylic outside type shit
    SDL_SetRenderDrawColor(g_renderer, 12, 13, 12, 255);
    SDL_RenderClear(g_renderer);

    // first fill all three LCD active areas with "off pixel" color
    SDL_SetRenderDrawColor(g_renderer, 174, 185, 125, 255);

    for (panel = 0; panel < PANEL_COUNT; panel++) {
        SDL_FRect rect;
        rect.x = panel_host_x(panel);
        rect.y = (float)DISPLAY_MARGIN;
        rect.w = (float)panel_host_width();
        rect.h = (float)panel_host_height();
        SDL_RenderFillRect(g_renderer, &rect);
    }

    // draw active/dark pixels
    SDL_SetRenderDrawColor(
        g_renderer,
        29, 36, 23, 255
    );

    for (y = 0; y < VIDEO_HEIGHT; y++) {
        for (x = 0; x < VIDEO_WIDTH; x++) {
            int current_panel;
            int local_x;
            SDL_FRect pixel;
            if (!vram_pixel(vram, x, y)) {
                continue;
            }
            current_panel = x / PANEL_WIDTH;
            local_x = x % PANEL_WIDTH;
            pixel.x = panel_host_x(current_panel) + (float)(local_x * DISPLAY_SCALE);
            pixel.y = (float)(DISPLAY_MARGIN + y * DISPLAY_SCALE);
            pixel.w = (float)DISPLAY_SCALE;
            pixel.h = (float)DISPLAY_SCALE;
            SDL_RenderFillRect(
                g_renderer,
                &pixel
            );
        }
    }

    // thin border around the panel for aesthetic type shit
    SDL_SetRenderDrawColor(g_renderer, 48, 52, 44, 255);
    for (panel = 0; panel < PANEL_COUNT; panel++) {
        SDL_FRect rect;
        rect.x = panel_host_x(panel);
        rect.y = (float)DISPLAY_MARGIN;
        rect.w = (float)panel_host_width();
        rect.h = (float)panel_host_height();
        SDL_RenderRect(g_renderer, &rect);
    }
    SDL_RenderPresent(g_renderer);
}