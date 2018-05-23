#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

    // TODO();
    
    t0 = cpu.cs;
    rtl_push(&cpu.eflags);
    rtl_push(&t0);
    rtl_push(&ret_addr);
    
    vaddr_t addr = NO * 8 + cpu.idtr.base;
    
    union {
        GateDesc tempGate;
        struct {
            uint32_t low_val;
            uint32_t high_val;
        };
    }temp;
    temp.low_val = vaddr_read(addr, 4);
    temp.high_val = vaddr_read(addr + 4, 4);
    decoding.is_jmp = 1;
    decoding.jmp_eip = (temp.tempGate.offset_15_0 & 0xffff) | ((temp.tempGate.offset_31_16 & 0xffff) << 16);
}

void dev_raise_intr() {
}
