#include <am.h>
#include <nemu.h>
#include <string.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

#define MASK_CODE 0x0000ffff

void __am_gpu_init() {
}

// --- 修复后 (更清晰的版本) ---
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    uint32_t vga_ctl = inl(VGACTL_ADDR);
    int w = (vga_ctl >> 16) & MASK_CODE;
    int h = vga_ctl & MASK_CODE;

    *cfg = (AM_GPU_CONFIG_T){
        .present = true,
        .has_accel = false,
        .width = w,
        .height = h,
        .vmemsz = w * h * sizeof(uint32_t) // 直接使用变量 w 和 h
    };
}

// --- 修复后 ---
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    if (ctl->pixels != NULL) {
        // ... (所有边界检查和 memcpy 的逻辑都放在这里)
        uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
        int w = (inl(VGACTL_ADDR) >> 16) & MASK_CODE;
        int x = ctl->x, y = ctl->y;
        int w_draw = ctl->w, h_draw = ctl->h;

        for (int j = 0; j < h_draw; j++) {
            uint32_t *fb_row = fb + (y + j) * w + x;
            uint32_t *pixels_row = (uint32_t *)ctl->pixels + j * w_draw;
            memcpy(fb_row, pixels_row, ctl->w * sizeof(uint32_t));
        }
    }

    // 同步操作必须在函数末尾执行，不受 pixels 是否为空的影响
    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}
