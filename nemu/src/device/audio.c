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
#include <device/map.h>
#include <SDL2/SDL.h>

// **主要作用：**
//
// 1. **寄存器索引映射**：为音频控制寄存器提供有意义的名称，避免使用魔数
//    - `audio_base[reg_freq]` 比 `audio_base[0]` 更有可读性
//
// 2. **内存布局定义**：确定音频控制寄存器在内存中的布局
//    - 每个寄存器占用4字节（uint32_t）
//    - 总共需要 `nr_reg * sizeof(uint32_t)` 字节空间
//
// 3. **硬件接口标准化**：定义了音频设备驱动与硬件交互的标准接口
//    - 软件可以通过写入不同寄存器来配置音频参数
//    - 例如：设置采样频率、声道数、初始化音频等
//
// 4. **代码维护性**：如果需要添加新的控制寄存器，只需在枚举中添加即可，`nr_reg` 会自动更新，空间分配代码无需修改

enum {
    reg_freq,      // 寄存器0：音频频率设置
    reg_channels,  // 寄存器1：音频声道数设置
    reg_samples,   // 寄存器2：音频采样位数设置
    reg_sbuf_size, // 寄存器3：音频缓冲区大小设置
    reg_init,      // 寄存器4：音频初始化控制
    reg_count,     // 寄存器5：音频计数器
    nr_reg         // 总寄存器数量（值为6）
};

static uint8_t *sbuf = NULL;        // 音频缓冲区指针
static uint32_t *audio_base = NULL; // 为音频控制寄存器分配的空间指针

static void audio_io_handler(uint32_t offset, int len, bool is_write) {}

void init_audio() {
    uint32_t space_size = sizeof(uint32_t) * nr_reg; // 计算音频控制寄存器的空间大小
    audio_base = (uint32_t *)new_space(space_size);  // 为音频控制寄存器分配空间(端口I/O映射)
#ifdef CONFIG_HAS_PORT_IO
    add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
    add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler); // 内存I/O映射
#endif

    sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);                            // 分配音频缓冲区空间
    add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL); // 分配音频缓冲区的内存I/O映射
}
