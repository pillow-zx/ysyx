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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;   // g_nr_guest_inst用于记录cpu运行的指令数
static uint64_t g_timer = 0;    // unit: us    // g_timer用于记录cpu运行的时间，默认为0
static bool g_print_step = false;   // g_print_step用于控制是否打印指令跟踪信息

void device_update();

static void trace_and_difftest(Decode* _this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
    /* 如果全局变量g_print_step被定义，输出跟踪的指令信息 */
    if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
    IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
}

/* 执行一次cpu的指令 */
static void exec_once(Decode* s, vaddr_t pc) {
    s->pc = pc;
    s->snpc = pc;
    isa_exec_once(s);
    cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
    char* p = s->logbuf;
    p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
    int ilen = s->snpc - s->pc;
    int i;
    uint8_t* inst = (uint8_t*)&s->isa.inst;
#ifdef CONFIG_ISA_x86
    for (i = 0; i < ilen; i++) {
#else
    for (i = ilen - 1; i >= 0; i--) {
#endif
        p += snprintf(p, 4, " %02x", inst[i]);
    }
    int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
    int space_len = ilen_max - ilen;
    if (space_len < 0) space_len = 0;
    space_len = space_len * 3 + 1;
    memset(p, ' ', space_len);
    p += space_len;

    void disassemble(char* str, int size, uint64_t pc, uint8_t * code, int nbyte);
    disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
        MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t*)&s->isa.inst, ilen);
#endif
}

/* 实际运行cpu指令 n为运行的指令数 */
static void execute(uint64_t n) {
    Decode s;
    for (;n > 0; n--) {
        // 实际运行cpu指令
        exec_once(&s, cpu.pc);
        g_nr_guest_inst++;
        trace_and_difftest(&s, cpu.pc);
        if (nemu_state.state != NEMU_RUNNING) break;
        IFDEF(CONFIG_DEVICE, device_update());
    }
}

/* 统计cpu的运行时间，运行的指令数 */
static void statistic() {
    IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
    /* 根据 CONFIG_TARGET_AM 的定义状态选择格式化字符串 */
    // 如果定义了 CONFIG_TARGET_AM，使用 "%"
    // 如果未定义，使用 "%'"（带千位分隔符的格式）
    // PRIu64 部分
    // 来自 <inttypes.h>
    // 用于 64 位无符号整数的可移植格式说明符
    // 展开为 "lu"（取决于平台)
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
    Log("host time spent = " NUMBERIC_FMT " us", g_timer);
    Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
    if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
    else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
    isa_reg_display();
    statistic();
}

/* Simulate how the CPU works. */
/* 模拟cpu运行 获取的n为cpu运行的指令数 */
void cpu_exec(uint64_t n) {
    /* 运行的指令数为负数时，表示一直运行到遇到异常或中断 */
    g_print_step = (n < MAX_INST_TO_PRINT);
    /* 判断nemu状态，如果处于未运行状态则停止对cpu的模拟 */
    switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT: case NEMU_QUIT:
        printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
        return;
    default: nemu_state.state = NEMU_RUNNING;
    }

    /* 获取cpu开始运行的时间 */
    uint64_t timer_start = get_time();

    /* 开始运行cpu */
    execute(n);

    /* 获取cpu结束运行的时间 */
    uint64_t timer_end = get_time();
    /* 得到cpu的运行时间 */
    g_timer += timer_end - timer_start;

    /* 检查nemu状态，结束nemu运行 */
    switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
        Log("nemu: %s at pc = " FMT_WORD,
            (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
                (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
                    ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
            nemu_state.halt_pc);
        // fall through
    case NEMU_QUIT: statistic();
    }
}
