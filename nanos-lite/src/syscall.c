#include "common.h"
#include "syscall.h"

static inline int sys_write(int fd, uint8_t *buf, size_t count){
    size_t len = 0;
    if(fd == 1 || fd == 2){
        while(++len < count){
            _putc(*buf);
            ++buf;
        }
        return len;
    }
    return -1;
}


_RegSet* do_syscall(_RegSet *r) {
    uintptr_t a[4];
    a[0] = SYSCALL_ARG1(r);
    a[1] = SYSCALL_ARG2(r);
    a[2] = SYSCALL_ARG3(r);
    a[3] = SYSCALL_ARG4(r);

    switch (a[0]) {
        case SYS_none: r->eax = 1; break;
        case SYS_exit: _halt(a[1]); break;
        case SYS_write: sys_write(a[1], (uint8_t*)a[2], a[3]); break;
        default: panic("Unhandled syscall ID = %d", a[0]);
    }

    return NULL;
}
