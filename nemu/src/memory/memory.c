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
        // /* this is a special case, you can handle it later. */
        // /* len of instruction 1 */
        // int len1 = PAGE_SIZE - (addr & 0x3ff);
        // /* len of instruction 2 */
        // int len2 = len - len1;
        // /* read instruction 1 and 2 addr from page table */
        // paddr_t addr1 = page_translate(addr, false);
        // paddr_t addr2 = page_translate(addr + len1, false);
        // /* read instruction 1 and 2 value from addr */
        // uint32_t val1 = paddr_read(addr1, len1);
        // uint32_t val2 = paddr_read(addr2, len2);
        // /* concat val1 and val2 */
        // return val1 | val2 << (len1 << 3);
        int len1 = 4096 - (addr & 0xfff);
        int len2 = len - len1;
        vaddr_t addr1 = page_translate(addr, false);
        vaddr_t addr2 = page_translate(addr + len1, false);
        uint32_t ret1 = paddr_read(addr1, len1);
        uint32_t ret2 = paddr_read(addr2, len2);
        uint32_t ans = ret1 | (ret2 << (len1 << 3));
        //printf("ans = %08X\n", ans);
        return ans;
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
        /* len of instruction 1 */
        int len1 = PAGE_SIZE - (addr & 0x3ff);
        /* len of instruction 2 */
        int len2 = len - len1;
        /* read instruction 1 and 2 addr from page table */
        paddr_t addr1 = page_translate(addr, true);
        paddr_t addr2 = page_translate(addr + len1, true);
        /* write data */
        paddr_write(addr1, len1, data & ((1 << (len1 << 3)) - 1));
        paddr_write(addr2, len2, data >> (len1 << 3));
    }
    else{
        paddr_t paddr = page_translate(addr, true);
        paddr_write(paddr, len, data);
    }
    // paddr_write(addr, len, data);
}
