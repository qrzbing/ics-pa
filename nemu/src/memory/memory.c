#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
    int map_No = is_mmio(addr);
    if (map_No == -1)
        return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
    else {
        return mmio_read(addr, len, map_No);
    }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
    int map_No = is_mmio(addr);
    if (map_No == -1)
        memcpy(guest_to_host(addr), &data, len);
    else {
        mmio_write(addr, len, data, map_No);
    }
}

paddr_t page_translate(vaddr_t addr, bool flag){
    paddr_t paddr = addr;
    /* only when protect mode and paging mode enable translate*/
    if(cpu.cr0.protect_enable && cpu.cr0.paging){
        /* initialize */
        paddr_t pdeptr, pteptr;
        PDE pde;
        PTE pte;
        /* find pde and read */
        pdeptr = (paddr_t)(cpu.cr3.page_directory_base << 12) | (paddr_t)(((addr >> 22) & 0x3ff) * 4);
        pde.val = paddr_read(pdeptr, 4);
        assert(pde.present);
        pde.accessed = 1;
        /* find pte and read */
        pteptr = (paddr_t)(pde.page_frame << 12) | (paddr_t)(((addr >> 12) & 0x3ff) * 4);
        pte.val = paddr_read(pteptr, 4);
        assert(pte.present);
        pte.accessed = 1;
        pte.dirty = (flag == true) ? 1 : 0;
        /* find page and read */
        paddr = (paddr_t)(pte.page_frame << 12) | (paddr_t)(addr & 0xfff);
    }
    return paddr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
    bool flag = ((addr + len - 1) & (~PAGE_MASK)) != (addr & (~PAGE_MASK));
    /* flag equals to true means data cross the page boundry */
    if(flag == true){
        /* this is a special case, you can handle it later. */
        Log("addr = %#x, len = %#x, cpu.cr0 = %#x", addr, len, cpu.cr0.val);
        panic("data cross the page boundry");
    }
    else{
        paddr_t paddr = page_translate(addr, false);
        return paddr_read(paddr, len);
    }
    // return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
    bool flag = ((addr + len - 1) & (~PAGE_MASK)) != (addr & (~PAGE_MASK));
    if(flag == true){
        Log("addr = %#x, len = %#x, cpu.cr0 = %#x", addr, len, cpu.cr0.val);
        panic("data cross the page boundry");
    }
    else{
        paddr_t paddr = page_translate(addr, true);
        paddr_write(paddr, len, data);
    }
    // paddr_write(addr, len, data);
}
