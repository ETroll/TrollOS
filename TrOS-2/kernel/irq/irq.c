#include <tros/hal/idt.h>
#include <tros/hal/gdt.h>
#include <tros/hal/pic.h>
#include <tros/tros.h>
#include <tros/irq.h>

static irq_handler __irq_handlers[MAX_IRQ];

static const char* irq_names[32] = {
	"Division by zero",
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Detected overflow",
	"Out-of-bounds",
	"Invalid opcode",
	"No coprocessor",
	"Double fault",
	"Coprocessor segment overrun",
	"Bad TSS",
	"Segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"Unknown interrupt",
	"Coprocessor fault",
	"Alignment check",
	"Machine check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void irq_initialize(void)
{
    for(int i = 0; i<MAX_IRQ; i++)
	{
		__irq_handlers[i] = 0;
	}

    gdt_initialize();
    idt_initialize();
    pic_initialize();
}

void irq_default_handler(cpu_registers_t regs)
{
	unsigned char itq_no = regs.irq_no & 0xFF;
	if (__irq_handlers[itq_no] != 0)
	{
		irq_handler handler = __irq_handlers[itq_no];
		handler(&regs);
	}
	else if(itq_no < 32)
	{
		// If we have a CPU exception and it is not handled
		// we are in touble.. Lets panic. (We did not bring a towel..)
		kernel_panic(irq_names[itq_no], &regs);
	}
	else
	{
		printk("Unhandled IRQ: %d\n", itq_no);
		if(itq_no == 33) printk("ACK! %d \n", itq_no-32);
		pic_eoi(itq_no-32);
	}
}

void irq_eoi(unsigned int irq)
{
	pic_eoi(irq);
}

int irq_register_handler(unsigned int irq, irq_handler handler)
{
    if(__irq_handlers[irq] != 0)
    {
    	return -1;
    }
    else
    {
        __irq_handlers[irq] = handler;
        return irq;
    }
}

void irq_remove_handler(unsigned int irq)
{
	__irq_handlers[irq] = 0;
}
