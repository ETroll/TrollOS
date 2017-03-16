// syscall.h
// Userland syscall implementation

#ifndef INCLUDE_SYS_SYSCALL_H
#define INCLUDE_SYS_SYSCALL_H

#define DECL_SYSCALL0(fn) int syscall_##fn();
#define DECL_SYSCALL1(fn,p1) int syscall_##fn(p1);
#define DECL_SYSCALL2(fn,p1,p2) int syscall_##fn(p1,p2);
#define DECL_SYSCALL3(fn,p1,p2,p3) int syscall_##fn(p1,p2,p3);
#define DECL_SYSCALL4(fn,p1,p2,p3,p4) int syscall_##fn(p1,p2,p3,p4);
#define DECL_SYSCALL5(fn,p1,p2,p3,p4,p5) int syscall_##fn(p1,p2,p3,p4,p5);

//Processes
DECL_SYSCALL0(getpid);
DECL_SYSCALL0(getparentpid);
DECL_SYSCALL2(execute, char*, char**);
DECL_SYSCALL1(exit, unsigned int);

//Devices
DECL_SYSCALL1(opendevice, char*);
DECL_SYSCALL1(closedevice, unsigned int);
DECL_SYSCALL3(writedevice, unsigned int, const void*, unsigned int);
DECL_SYSCALL3(readdevice, unsigned int, void*, unsigned int);
DECL_SYSCALL3(ioctl, unsigned int, unsigned int, unsigned int);

//Memory
DECL_SYSCALL1(increasemem, unsigned int);
DECL_SYSCALL1(decreasemem, unsigned int);

DECL_SYSCALL1(debug, unsigned int);

#endif
