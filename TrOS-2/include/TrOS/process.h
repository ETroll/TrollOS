// Process.h
// Multiasking functionality

#ifndef INCLUDE_TROS_TASK_H
#define INCLUDE_TROS_TASK_H

#include <tros/vmm.h>

typedef struct
{
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int esp;
    unsigned int ebp;
    unsigned int eip;
    unsigned int eflags;
    unsigned int cr3;
} __attribute__((packed)) registers_t;

typedef struct
{
    unsigned int user_stack_ptr;
    unsigned int kernel_stack_ptr;
    unsigned int instr_ptr;
    int priority;
    int state;
} thread_t;

typedef struct process
{
    thread_t thread;
    registers_t regs;
    pdirectory_t* pagedir;
    struct process *next;
    unsigned int pid;
} process_t;

// void process_init();
extern void process_switch(registers_t* old, registers_t* new);
// void process_create(process_t* task, void(*main)(), unsigned int flags, unsigned int* pagedir);

void process_preempt();
void process_switchto(process_t* next);
void process_create_idle(void (*main)());

void process_exec_user(unsigned int startAddr);

#endif
