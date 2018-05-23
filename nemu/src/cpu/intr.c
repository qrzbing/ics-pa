#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

    TODO();
    rtl_push(&cpu.eflags);
    t0 = cpu.cs;
    rtl_push(&t0);
    rtl_push(&ret_addr);
}

void dev_raise_intr() {
}
