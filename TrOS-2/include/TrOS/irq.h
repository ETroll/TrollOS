// irq.h
// Main handling of IRQs both software and hardware

#ifndef INCLUDE_TROS_IRQ_H
#define INCLUDE_TROS_IRQ_H

#include <stdint.h>

#define MAX_IRQ 256

typedef struct registers
{
    uint32_t ds; // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t irq_no, err_code; // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} irq_registers_t;

typedef void (*irq_handler)(irq_registers_t);

void irq_initialize(void);
void irq_default_handler();
int irq_handler_register(unsigned int irq, irq_handler handler);

#endif