// Kernel.c
// TrOS kernel - Main entry

#include <tros/tros.h>
#include <tros/timer.h>
#include <tros/irq.h>
// #include <tros/pmm.h>	// REMOVE
// #include <tros/vmm.h> 	// REMOVE
#include <tros/fs/vfs.h>
// #include <tros/memory.h>
#include <tros/exec.h>
#include <tros/memory.h>
#include <tros/hwdetect.h>
#include <tros/sys/multiboot.h>

#include <tros/process.h>

#include <tros/klib/kstring.h>

//Drivers baked in to the kernel
extern int kbd_driver_initialize();
extern int floppy_driver_initialize(unsigned char device);
extern int vga_driver_initialize();

//Filesystems baked in to the kernel
extern int fat12_fs_initialize();

//other, maybe use a correct header file?
extern void syscall_initialize();
extern void serial_init();

void kernel_idle();

// #define UNDERDEV 1

void kernel_early()
{
    irq_initialize();
    serial_init();
    timer_initialize(50);

    __asm("sti");
}

void kernel_memory_initialize(uint32_t stack_top, multiboot_info_t* multiboot)
{
    uint32_t memSize = 1024 + multiboot->memoryLo + multiboot->memoryHi*64;
    mem_usage_physical_t usage;

    memory_initialize(stack_top, memSize, 0x1000);
    memory_physical_usage(&usage);
    printk("Kernel stack top at: %x \n", stack_top);
    printk("\nBlocks initialized: %d\nUsed or reserved blocks: %d\nFree blocks: %d\n\n",
        usage.total,
        usage.used,
        usage.free);
}

void kernel_drivers()
{
    hwd_floppy_t fd = hwdetect_floppy_disks();
    if(fd.master > 0)
    {
        floppy_driver_initialize(0) ? printk("OK\n") : printk("FAILED!\n");
    }
    if(fd.slave > 0)
    {
        floppy_driver_initialize(1) ? printk("OK\n") : printk("FAILED!\n");
    }

    kbd_driver_initialize() ? printk("OK\n") : printk("FAILED!\n");
    vga_driver_initialize() ? printk("OK\n") : printk("FAILED!\n");
}

void kernel_filesystems()
{
    vfs_initialize();
    fat12_fs_initialize();
}

void kernel_main(multiboot_info_t* multiboot, uint32_t magic, uint32_t stack_top)
{
    kernel_early();
    kernel_memory_initialize(stack_top, multiboot);


    kernel_drivers();
    kernel_filesystems();

    /*
        fd0/
            initrd
            kernel.elf
            krnldr.bin
            test
            folder/
                test1
                test2
                ...
        hd0/
        cd0/
        dvd0/
    */

    if(!vfs_mount("fd0", "fat12"))
    {
        kernel_panic("Error mounting root folder. Halting!", 0);
    }

    syscall_initialize();
    printk("\n\n");
#ifndef UNDERDEV
    process_create_idle(&kernel_idle);

    char* argv[] =
    {
        "/fd0/bin/trell"
    };
    exec_elf32(argv[0], 1, argv);
#endif

    while(1)
    {
        kernel_panic("Reached END OF KERNEL", 0);
    }
}

// This is the "idle process"
void kernel_idle()
{
    while(1)
    {
        __asm("sti");
        __asm("hlt;");
    }
}
