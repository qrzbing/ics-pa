#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

    // TODO();
    rtl_push(&cpu.eflags);
        t0 = cpu.cs;
        rtl_push(&t0);;
        rtl_push(&ret_addr);
        uint32_t target_eip_low, target_eip_high;
        target_eip_low = vaddr_read(cpu.idtr.base + NO * 8, 4) & 0x0000ffff;
        target_eip_high = vaddr_read(cpu.idtr.base + NO * 8 + 4, 4) & 0xffff0000;
       decoding.is_jmp = 1;   
       decoding.jmp_eip = target_eip_low | target_eip_high;
}

void dev_raise_intr() {
}
