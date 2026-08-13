#ifndef CPU_H
#define CPU_H

#include <stdint.h>

int cpu_init(void);
void cpu_shutdown(void);

void cpu_reset(void);

void cpu_run_slice(uint64_t instructions);

#endif