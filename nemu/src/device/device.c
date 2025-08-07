/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <utils.h>
#include <device/alarm.h>
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>
#endif

void init_map();    // 初始化 I/O 映射空间
void init_serial(); // 初始化串口设备
void init_timer();
void init_vga();    // 初始化 VGA 设备
void init_i8042();  // 初始化 i8042 设备
void init_audio();  // 初始化音频设备
void init_disk();   // 初始化磁盘设备
void init_sdcard(); // 初始化 SD 卡设备
void init_alarm();  // 初始化定时器设备

void send_key(uint8_t, bool);
void vga_update_screen();

/* 设备更新 */
void device_update() {
    static uint64_t last = 0;
    uint64_t now = get_time();
    /* 设备更新时间间隔达到167时才会更新 */
    if (now - last < 1000000 / TIMER_HZ) {
        return;
    }
    last = now; // 更新上次更新时间

    IFDEF(CONFIG_HAS_VGA, vga_update_screen()); // 更新 VGA 屏幕

#ifndef CONFIG_TARGET_AM
    SDL_Event event; // SDL 事件处理
    /* SDL_PollEvent 函数用于轮询当前挂起的事件。
     * 如果参数 event 不为 NULL，则从事件队列中移除下一个事件并将其存储到 SDL_Event 结构中；
     * 如果 event 为 NULL，则仅检查队列中是否有事件而不移除。
     * 此函数可能隐式调用 SDL_PumpEvents()，
     * 因此只能在设置视频模式的线程中调用，通常用于主循环中以非阻塞方式处理系统事件。
     */
    while (SDL_PollEvent(&event)) {
        /* 检查事件类型 */
        switch (event.type) {
            case SDL_QUIT: nemu_state.state = NEMU_QUIT; break;
#ifdef CONFIG_HAS_KEYBOARD
            /* 如果按下了键盘按键
             * SDL_KEYDOWN 事件表示按键被按下，SDL_KEYUP 事件表示按键被释放。
             * event.key.keysym.scancode 是按键的扫描码
             * event.key.type 是事件类型，SDL_KEYDOWN 或 SDL_KEYUP。
             * send_key 函数用于处理按键事件，传入扫描码和按键状态（按下或释放）。
             */
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                uint8_t k = event.key.keysym.scancode;
                bool is_keydown = (event.key.type == SDL_KEYDOWN);
                send_key(k, is_keydown);
                break;
            }
#endif
            default: break;
        }
    }
#endif
}

/* 清空 SDL 事件队列 */
void sdl_clear_event_queue() {
#ifndef CONFIG_TARGET_AM
    SDL_Event event;
    while (SDL_PollEvent(&event))
        ;
#endif
}

/* 初始化设备 */
void init_device() {
    IFDEF(CONFIG_TARGET_AM, ioe_init()); // 初始化 I/O 设备
    init_map();                          // 初始化映射空间

    IFDEF(CONFIG_HAS_SERIAL, init_serial());  // 初始化串口设备
    IFDEF(CONFIG_HAS_TIMER, init_timer());    // 初始化时钟设备
    IFDEF(CONFIG_HAS_VGA, init_vga());        // 初始化 VGA 设备
    IFDEF(CONFIG_HAS_KEYBOARD, init_i8042()); // 初始化 i8042 控制器设备 (ps/2 键盘)
    IFDEF(CONFIG_HAS_AUDIO, init_audio());    // 初始化音频设备
    IFDEF(CONFIG_HAS_DISK, init_disk());      // 初始化磁盘设备
    IFDEF(CONFIG_HAS_SDCARD, init_sdcard());  // 初始化 SD 卡设备

    IFNDEF(CONFIG_TARGET_AM, init_alarm()); // 初始化定时器设备
}
