#include "common.h"
#include "fs.h"

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);

#define DEFAULT_ENTRY ((void *)0x4000000)
uintptr_t loader(_Protect *as, const char *filename) {
    // TODO();
    // size_t len = get_ramdisk_size();
    // ramdisk_read(DEFAULT_ENTRY, 0, len);
    
    int fd = fs_open(filename, 0, 0);
    size_t len = fs_filesz(fd);
    Log("LOAD [%d] %s. Size:%d", fd, filename, len);
    fs_read(fd, DEFAULT_ENTRY, len);
    return (uintptr_t)DEFAULT_ENTRY;
}
