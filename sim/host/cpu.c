#include "cpu.h"
#include "machine.h"
#include "libx86emu/include/x86emu.h"
#include <stdint.h>
#include <stdio.h>

static x86emu_t *g_cpu = NULL;

// mem helpers
static uint32_t mem_read_width(uint32_t addr, unsigned width)
{
    switch (width) {
        case X86EMU_MEMIO_16:
            return (uint32_t)machine_mem_read8(addr) | ((uint32_t)machine_mem_read8(addr + 1) << 8);
        case X86EMU_MEMIO_32:
            return (uint32_t)machine_mem_read8(addr) | ((uint32_t)machine_mem_read8(addr + 1) << 8) | ((uint32_t)machine_mem_read8(addr + 2) << 16) | ((uint32_t)machine_mem_read8(addr + 3) << 24);
        case X86EMU_MEMIO_8:
        case X86EMU_MEMIO_8_NOPERM:
        default:
            return machine_mem_read8(addr);
    }
}

static void mem_write_width(
    uint32_t addr,
    uint32_t value,
    unsigned width
)
{
    switch (width) {
        case X86EMU_MEMIO_16:
            machine_mem_write8(addr, value & 0xFF);
            machine_mem_write8(addr + 1, (value >> 8) & 0xFF);
            break;
        case X86EMU_MEMIO_32:
            machine_mem_write8(addr, value & 0xFF);
            machine_mem_write8(addr + 1, (value >> 8) & 0xFF);
            machine_mem_write8(addr + 2, (value >> 16) & 0xFF);
            machine_mem_write8(addr + 3, (value >> 24) & 0xFF);
            break;
        case X86EMU_MEMIO_8:
        case X86EMU_MEMIO_8_NOPERM:
        default:
            machine_mem_write8(addr, value & 0xFF);
            break;
    }
}

// io helpers
static uint32_t io_read_width(uint16_t port, unsigned width)
{
    switch (width) {
        case X86EMU_MEMIO_16:
            return (uint32_t)machine_io_read8(port) | ((uint32_t)machine_io_read8((uint16_t)(port + 1)) << 8);
        case X86EMU_MEMIO_32:
            return (uint32_t)machine_io_read8(port) | ((uint32_t)machine_io_read8((uint16_t)(port + 1)) << 8) | ((uint32_t)machine_io_read8((uint16_t)(port + 2)) << 16) | ((uint32_t)machine_io_read8((uint16_t)(port + 3)) << 24);
        default:
            return machine_io_read8(port);
    }
}

static void io_write_width(
    uint16_t port,
    uint32_t value,
    unsigned width
)

{
    switch (width) {
        case X86EMU_MEMIO_16:
            machine_io_write8(port, value & 0xFF);
            machine_io_write8((uint16_t)(port + 1), (value >> 8) & 0xFF);
            break;
        case X86EMU_MEMIO_32:
            machine_io_write8(port, value & 0xFF);
            machine_io_write8((uint16_t)(port + 1), (value >> 8) & 0xFF);
            machine_io_write8((uint16_t)(port + 2), (value >> 16) & 0xFF);
            machine_io_write8((uint16_t)(port + 3), (value >> 24) & 0xFF);
            break;
        default:
            machine_io_write8(port, value & 0xFF);
            break;
    }
}

// libx86emu to grip bus bridge
static unsigned grip_memio
(
    x86emu_t *emu,
    uint32_t addr,
    uint32_t *value,
    unsigned type
)
{
    (void)emu;
    unsigned width = type & 0xFF;
    unsigned operation = type & ~0xFFu;
    switch (operation) {
        // normal memread
        case X86EMU_MEMIO_R:
            *value = mem_read_width(addr, width);
            break;
        // instruction fetch
        case X86EMU_MEMIO_X:
            *value = mem_read_width(addr, width);
            break;
        // memwrite
        case X86EMU_MEMIO_W:
            mem_write_width(addr, *value, width);
            break;
        // in
        case X86EMU_MEMIO_I:
            *value = io_read_width((uint16_t)addr, width);
            break;
        // out
        case X86EMU_MEMIO_O:
            io_write_width((uint16_t)addr, *value, width);
            break;
        default:
            break;
    }
    return 0;
}

// cpu lifecycle
int cpu_init(void)
{
    g_cpu = x86emu_new(X86EMU_PERM_RWX, X86EMU_PERM_RW);
    if (g_cpu == NULL) {
        fprintf(stderr, "cpu: x86emu_new failed\n");
        return -1;
    }
    x86emu_set_memio_handler(g_cpu, grip_memio);
    cpu_reset();
    return 0;
}

void cpu_shutdown(void)
{
    if (g_cpu != NULL) {
        g_cpu = x86emu_done(g_cpu);
    }
}

void cpu_reset(void)
{
    if (g_cpu == NULL)
        return;
    x86emu_reset(g_cpu);
    printf("cpu: reset -> F000:FFF0 (physical FFFF0h)\n");
}

void cpu_run_slice(uint64_t instructions)
{
    if (g_cpu == NULL)
        return;
    g_cpu->max_instr = instructions;
    x86emu_run(g_cpu, X86EMU_RUN_MAX_INSTR | X86EMU_RUN_LOOP);
}