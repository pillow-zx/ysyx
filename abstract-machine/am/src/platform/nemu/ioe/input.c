#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint16_t key = (uint16_t)inl(KBD_ADDR);
    kbd->keydown = (key & KEYDOWN_MASK) != 0;  // 检查按键是否按下
    kbd->keycode = key & ~KEYDOWN_MASK;  // 获取按键代码
}
