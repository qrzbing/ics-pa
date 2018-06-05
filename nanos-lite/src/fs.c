#include "fs.h"

typedef struct {
    char *name;
    size_t size;
    off_t disk_offset;
    off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode){
    int fd;
    for(fd = 0; fd < NR_FILES; ++fd){
        if(strcmp(pathname, file_table[fd].name) == 0){
            break;
        }
    }
    if(fd >= NR_FILES) panic("file not found!");
    file_table[fd].open_offset = 0;
    return fd;
}

void ramdisk_read(void *buf, off_t offset, size_t len);
ssize_t fs_read(int fd, void *buf, size_t len){
    
    Finfo *fp = &file_table[fd];
    
    if(fp->size - fp->open_offset < len){
        len = fp->size - fp->open_offset;
    }

    switch(fd){
        case FD_STDOUT: case FD_STDERR:
            return -1;
        default:
            ramdisk_read(buf, fp->disk_offset + fp->open_offset, len);
            fp->open_offset += len;
        return len;
    }
}

void ramdisk_write(const void *buf, off_t offset, size_t len);
ssize_t fs_write(int fd, uint8_t *buf, size_t len){
    
    Finfo *fp = &file_table[fd];

    if(fp->size - fp->open_offset < len){
        len = fp->size - fp->open_offset;
    }

    size_t i = 0;
    switch(fd){
        case FD_STDIN: return -1;
        case FD_STDOUT: case FD_STDERR:
            while(i++ < len) _putc(*buf++);
            return len;
        default:
            ramdisk_write(buf, fp->disk_offset + fp->open_offset, len);
            fp->open_offset += len;
            return len;
    }
}

off_t fs_lseek(int fd, off_t offset, int whence){
    
    Finfo *fp = &file_table[fd];
    
    switch(whence){
        case SEEK_SET:
            break;
        case SEEK_CUR:
            offset = fp->open_offset + offset;
            break;
        case SEEK_END:
            offset = fp->size + offset;
            break;
        default: return -1;
    }
    if(offset < 0 || offset > fp->size) return -1;
    fp->open_offset = offset;
    return fp->open_offset;
}

int fs_close(int fd){
    return 0;
}

size_t fs_filesz(int fd){
    return file_table[fd].size;
}
