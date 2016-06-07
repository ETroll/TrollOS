// // Trell
// // A _highly_ inefficient half arsed excuse for a shell
// // Disclaimwer: Mostly written while drunk one friday night...
// // NOTE TO SOBER SELF: Rewrite stuff..!
//
// #include <trell/trell.h>
// #include <tros/tros.h>
// #include <sys/keyboard.h>
//
// #include <string.h>
// #include <stdio.h>
// #include <syscall.h>
//
// #define VGA_COLS 80
// #define TRELL_RCOLUMN_SIZE 11
// #define TRELL_CONSOLE_COLS VGA_COLS // - TRELL_RCOLUMN_SIZE
// #define TRELL_CONSOLE_ROWS 25
//
// #define TRELL_MAX_HISTORY 400
//
// static char __trell_history[TRELL_MAX_HISTORY][TRELL_CONSOLE_COLS];
// static unsigned int _wierd_alignment_bug;   //TODO! Big TODO! Figure out
//
// static unsigned int __trell_history_current;
// static unsigned int __trell_history_top;
// static unsigned int __trell_history_bottom;
// static unsigned int __trell_line_pos;
//
// static int __trell_vga;
//
// static char curr_working_dir[255];
//
// static void trell_cmd();
// void trell_clear();
//
// void trell_initialize()
// {
//     _wierd_alignment_bug = 0;
//     __trell_history_current = 0;
//     __trell_history_top = 0;
//     __trell_history_bottom = 0;
//     __trell_line_pos = 0;
//
//     for(int i = 0; i<TRELL_MAX_HISTORY; i++)
//     {
//         memset(__trell_history[i], 0x20, TRELL_CONSOLE_COLS);
//     }
//
//     __trell_vga = syscall_open("vga");
//
//     //Lets get information allready on screen and save it
//     syscall_ioctl(__trell_vga, IOCTL_VGA_TOGGLE_CURSOR, 0);
//     syscall_ioctl(__trell_vga, IOCTL_VGA_SHOULD_SCROLL, 0);
//
//     for(int pos = 0; pos < VGA_COLS*(TRELL_CONSOLE_ROWS-1);
//         pos+=VGA_COLS)
//     {
//         syscall_seek(__trell_vga, pos);
//         syscall_read(__trell_vga, __trell_history[__trell_history_bottom++], TRELL_CONSOLE_COLS);
//     }
//
//     //Lets find our first free line from bottom-up
//     int last_sum = 0;
//     for(; __trell_history_bottom > __trell_history_top; __trell_history_bottom--)
//     {
//         int sum = 0;
//         for(int pos = 0; pos<TRELL_CONSOLE_COLS; pos++)
//         {
//             sum += (__trell_history[__trell_history_bottom][pos] - 0x20);
//         }
//
//         if(sum != 0 && last_sum == 0)
//         {
//             break;
//         }
//         else
//         {
//             last_sum = sum;
//         }
//     }
//
//     __trell_history_bottom++;
//
//     syscall_ioctl(__trell_vga, IOCTL_VGA_TOGGLE_CURSOR, 1);
//     syscall_seek(__trell_vga, __trell_history_bottom*VGA_COLS);
//
//     __trell_history_current = __trell_history_bottom;
//
//     //printk("trell_history at %x", &__trell_history);
//     //strcpy(curr_working_dir, "/");
//     //trell_cmd();
//
//     while(1) {
//         syscall_write(__trell_vga, 'X', 1);
//     }
//
//     syscall_close(__trell_vga);
// }
//
// void trell_clear()
// {
//     __trell_history_top = 0;
//     __trell_history_bottom = 0;
//     __trell_history_current = 0;
//     __trell_line_pos = 0;
//     syscall_ioctl(__trell_vga, IOCTL_VGA_CLEAR_MEM, 0);
// }
//
// void trell_refresh()
// {
//     int seekpos = 0;
//     for(int i = __trell_history_top; i<__trell_history_bottom+1; i++)
//     {
//         syscall_seek(__trell_vga, seekpos);
//         syscall_write(__trell_vga, __trell_history[i], TRELL_CONSOLE_COLS);
//         seekpos+=VGA_COLS;
//     }
//
//     if(__trell_history_current - __trell_history_top >= TRELL_CONSOLE_ROWS-1)
//     {
//         syscall_seek(__trell_vga, VGA_COLS*(TRELL_CONSOLE_ROWS-1)+__trell_line_pos);
//     }
//     else
//     {
//         syscall_seek(__trell_vga, VGA_COLS*(__trell_history_bottom-__trell_history_top+__trell_line_pos));
//     }
//     syscall_ioctl(__trell_vga, IOCTL_VGA_TOGGLE_CURSOR, 1);
// }
//
// void trell_newline()
// {
//     __trell_history_current++;
//     __trell_line_pos = 0;
//
//     if(__trell_history_current - __trell_history_top >= TRELL_CONSOLE_ROWS)
//     {
//         if(__trell_history_current >= TRELL_MAX_HISTORY)
//         {
//             //Remove 100 from history
//             //copy data to new location
//             //set history pointers
//         }
//
//         __trell_history_top++;
//         __trell_history_bottom++;
//
//         syscall_ioctl(__trell_vga, IOCTL_VGA_SCROLL_UP, 1);
//         syscall_seek(__trell_vga, VGA_COLS*(TRELL_CONSOLE_ROWS-1));
//     }
//     else
//     {
//         syscall_seek(__trell_vga, VGA_COLS*(__trell_history_bottom-__trell_history_top));
//     }
// }
//
// void trell_putch(char c)
// {
//     //NOTE: If we are not displaying the last entries, then scroll our collective
//     //      arses down to the last entries.
//     //      - Seek to correct location for next char
//     //      - Move history back to reality
//
//     if(__trell_history_bottom != __trell_history_current)
//     {
//         __trell_history_bottom = __trell_history_current;
//         if(__trell_history_bottom-TRELL_CONSOLE_ROWS > 0)
//         {
//             __trell_history_top = __trell_history_bottom - (TRELL_CONSOLE_ROWS-1);
//         }
//         else
//         {
//             __trell_history_top = 0;
//         }
//         trell_refresh();
//     }
//
//     if (c == '\n'|| c == '\r' || __trell_line_pos > (TRELL_CONSOLE_COLS-1))
//     {
//         trell_newline();
//     }
//     else
//     {
//         __trell_history[__trell_history_current][__trell_line_pos++] = c;
//         syscall_write(__trell_vga, &c, 1);
//     }
// }
// void trell_puts(const char* str)
// {
//     while (*str)
//     {
//         trell_putch(*str);
//         str++;
//     }
// }
//
// // static void otherMain() {
// //     while(1) {
// //         printk("Hello multitasking world!\n");
// //         process_preempt();
// //     }
// // }
//
// void kernel_run_command(char* cmd)
// {
// 	char* argv[5];	//NOTE: Hardcoded a maximum of 5 params for now
// 	unsigned int argc = 1;
//
// 	argv[0] = cmd;
// 	while(*cmd++ != '\0' && argc < 5)
// 	{
// 		if(*cmd == ' ')
// 		{
// 			*cmd = '\0';
// 			argv[argc++] = ++cmd;
// 		}
// 	}
// 	// printk("Got command %s with %d parameters\n", argv[0], argc-1);
// 	// for(int i=1; i<argc; i++)
// 	// {
// 	// 	printk("parameter %d: %s\n", i, argv[i]);
// 	// }
//
// 	if(strcmp(argv[0], "clr") == 0)
// 	{
//         trell_clear();
// 	}
// 	else if(strcmp(argv[0], "help") == 0)
// 	{
//         printk("TrOS-2 Help:\n");
//         printk("Commands:\n");
//         printk(" - help: Displays this help message\n");
//         printk(" - clr:  Clears the display\n");
//         //printk(" - rs <sect>:  Reads the sector data from floppy\n");
//         printk(" - memtest:  Runs a primitive memory test\n");
//         printk(" - mmap:  Lists taken and not taken physical blocks\n");
//         printk(" - usermode:  flips the CPU to ring0\n");
//         printk(" - ls/cd/pwd:  list and navigate FS\n");
//         printk(" - exec:  Executes a test process\n");
// 	}
//     // else if(strcmp(argv[0], "memtest") == 0)
// 	// {
//     //     if(argc > 1)
//     //     {
//     //         void* data = kmalloc(atoi(argv[1]));
//     //         printk("Got data allocated at %x\n", data);
//     //     }
//     //     else
//     //     {
//     //         printk("--- alloc 5000:\n");
//     //         void* data = kmalloc(5000);
//     //         printk("--- alloc 100:\n");
//     //         void* stuff = kmalloc(100);
//     //         printk("--- alloc 400:\n");
//     //         void* morestuff = kmalloc(400);
//     //
//     //         printk("--- free 100:\n");
//     //         kfree(stuff);
//     //         printk("--- free 400:\n");
//     //         kfree(morestuff);
//     //         printk("--- free 5000:\n");
//     //         kfree(data);
//     //
//     //         printk("--- alloc 100:\n");
//     //         data = kmalloc(100);
//     //         printk("--- alloc 300:\n");
//     //         stuff = kmalloc(300);
//     //         printk("Got data allocated at %x and stuff at %x\n", data, stuff);
//     //
//     //     }
//     //
// 	// }
//     // else if(strcmp(argv[0], "mmap") == 0)
// 	// {
//     //     if(argc > 1)
//     //     {
//     //         unsigned int num_blocks = 32;
//     //         unsigned int start_block = atoi(argv[1]);
//     //         if(argc == 3)
//     //         {
//     //             num_blocks = atoi(argv[2]);
//     //         }
//     //
//     //         for(int block = start_block; block<(num_blocks+start_block); block++)
//     //         {
//     //             int taken = mmap_test_block(block);
//     //             unsigned int addr = block * 4096;
//     //             printk("Block: %x is %s\n", addr, taken == 0 ? "FREE" :"TAKEN");
//     //         }
//     //     }
//     //     else
//     //     {
//     //         printk("Usage: mmap <startblock> (<numblocks>)\n");
//     //     }
//     // }
//     // else if(strcmp(argv[0], "exec") == 0)
//     // {
//     //     process_init();
//     //     for(int i = 0; i<2; i++) {
//     //         process_preempt();
//     //         printk(" Returned to mainTask!\n");
//     //     }
//     // }
//     // else if(strcmp(argv[0], "ls") == 0)
//     // {
//     //     fs_node_t* dir = kopen(curr_working_dir);
//     //     printk("Dir %s at %x\n", curr_working_dir, dir);
//     //     unsigned int index = 0;
//     //
//     //     dirent_t* dirent = vfs_readdir(dir, index);
//     //     while (dirent != 0)
//     //     {
//     //         printk("Dirent: %x - %s\n", dirent, dirent->name);
//     //         //kfree(dirent);
//     //
//     //         index++;
//     //         dirent = vfs_readdir(dir, index);
//     //     }
//     //     vfs_close(dir);
//     //     kfree(dir); //Not freed in VFS yet..
//     // }
//     // else if(strcmp(argv[0], "cd") == 0)
//     // {
//     //     if(argc > 1)
//     //     {
//     //         fs_node_t* dir = kopen(argv[1]);
//     //         if(dir)
//     //         {
//     //             if(dir->flags & VFS_FLAG_DIRECTORY)
//     //             {
//     //                 strcpy(curr_working_dir, argv[1]);
//     //                 vfs_close(dir);
//     //             }
//     //             else
//     //             {
//     //                 printk("%s is not a directory\n", argv[1]);
//     //             }
//     //         }
//     //     }
//     // }
//     // else if(strcmp(argv[0], "pwd") == 0)
//     // {
//     //     printk("%s\n", curr_working_dir);
//     // }
//     else
//     {
//     	printk("Unknown command\n");
//     }
// }
//
// static void trell_cmd()
// {
// 	char cmd_buffer[100];
// 	int buffer_loc = 0;
// 	int next_command = 0;
// 	int key;
//
//     int kbd = syscall_open("kbd");
// 	while(1)
// 	{
// 		buffer_loc = 0;
// 		next_command = 0;
//
// 		printk("> ");
//
// 		while(!next_command)
// 		{
// 			key = 0;
// 			if(syscall_read(kbd, &key, 1))
// 			{
// 				if(key != KEY_LCTRL
// 					&& key != KEY_RCTRL
// 					&& key != KEY_RALT
// 					&& key != KEY_LALT
// 					&& key != KEY_RSHIFT
// 					&& key != KEY_LSHIFT
// 					&& key != KEY_CAPSLOCK)
// 				{
// 					if(key == KEY_RETURN)
// 					{
// 						cmd_buffer[buffer_loc] = '\0';
// 						printk("\n");
// 						kernel_run_command(cmd_buffer);
// 						next_command = 1;
// 					}
//                     else if(key == KEY_PAGEUP)
// 					{
//                         if(__trell_history_top != 0)
//                         {
//                             __trell_history_top--;
//                             __trell_history_bottom--;
//
//                             syscall_ioctl(__trell_vga, IOCTL_VGA_TOGGLE_CURSOR, 0);
//                             syscall_ioctl(__trell_vga, IOCTL_VGA_SCROLL_DOWN, 1);
//
//                             syscall_seek(__trell_vga, 0);
//                             syscall_write(__trell_vga, __trell_history[__trell_history_top], TRELL_CONSOLE_COLS);
//                         }
//                     }
//                     else if(key == KEY_PAGEDOWN)
// 					{
//                         printk("PGDOWN\n");
//                     }
//                     else if(key == KEY_UP)
// 					{
//                         printk("UP\n");
//                     }
//                     else if(key == KEY_DOWN)
// 					{
//                         printk("DOWN\n");
//                     }
// 					else if(key == KEY_BACKSPACE)
// 					{
// 						if(buffer_loc > 0)
// 						{
// 							cmd_buffer[buffer_loc-1] = ' ';
// 							buffer_loc--;
//
// 							// vga_position_t pos = vga_get_position();
// 							// pos.x--;
// 							// vga_set_position(pos.x, pos.y);
// 							// vga_putch(' ');
// 							// vga_set_position(pos.x, pos.y);
// 						}
// 					}
// 					else
// 					{
// 						if(buffer_loc < 100)
// 						{
// 							cmd_buffer[buffer_loc++] = key;
// 							trell_putch((char)key);
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
//     syscall_close(kbd);
// }