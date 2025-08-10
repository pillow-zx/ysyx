#include <am.h>
#include <klib.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00) // 音频频率地址
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04) // 音频通道地址
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08) // 音频采样地址
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c) // 音频缓冲区地址
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10) // 音频初始化地址
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14) // 音频计数地址

void __am_audio_init() {}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
    outl(AUDIO_FREQ_ADDR, ctrl->freq);
    outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
    outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
    outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
    stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
    uint8_t *start = ctl->buf.start;
    int size = ctl->buf.end - ctl->buf.start;
    int buf_size = inl(AUDIO_SBUF_SIZE_ADDR);
    
    // 等待有足够空间
    int current_count;
    do {
        current_count = inl(AUDIO_COUNT_ADDR);
    } while (size > buf_size - current_count);
    
    // 处理环形缓冲区
    int write_pos = current_count % buf_size;
    uint8_t *dest = (uint8_t *)AUDIO_SBUF_ADDR + write_pos;
    
    if (write_pos + size <= buf_size) {
        // 一次性写入
        memcpy(dest, start, size);
    } else {
        // 分两次写入（处理环绕）
        int first_part = buf_size - write_pos;
        memcpy(dest, start, first_part);
        memcpy((uint8_t *)AUDIO_SBUF_ADDR, start + first_part, size - first_part);
    }
    
    outl(AUDIO_COUNT_ADDR, current_count + size);
}