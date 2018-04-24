#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
    return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
    int i, j;
    //int cp_bytes = sizeof(uint32_t) * w;
    //for (i = 0; i < _screen.width * _screen.height; i++) {
    //for (i = 0; i < h; i++){
        //memcpy(&fb[(y + i) * w + x], pixels, cp_bytes);
        //fb[i + 400 * 100] = pixels[i];
    //}
    for(i = y; i < y + h; ++i){
        for(j = x; j < x + w; ++j){
            fb[i * _screen.width + j] = pixels[(i - y) * w + (j - x)];
        }
    }
}
void _draw_sync() {}

// keyboard input
#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
#define I8042_STATUS_HASKEY_MASK 0x1
#define KEYBOARD_IRQ 1
int _read_key() {
    int keyvalue = _KEY_NONE;
    if((inb(I8042_STATUS_PORT) & 0x1) == 1){
        keyvalue = inl(I8042_DATA_PORT);
    }
    //return _KEY_NONE;
    return keyvalue;
}
