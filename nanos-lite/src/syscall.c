#include "common.h"
#include "syscall.h"

_RegSet* do_syscall(_RegSet *r) {
    uintptr_t a[4];
    a[0] = SYSCALL_ARG1(r);

    switch (a[0]) {
        case SYS_none: r->eax = 1; break;
        case SYS_exit: _halt(r->eax); break;
        default: panic("Unhandled syscall ID = %d", a[0]);
    }

    return NULL;
}
