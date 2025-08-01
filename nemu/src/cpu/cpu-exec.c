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

#include "utils.h"
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

// 个人添加的头文件
#include <../../monitor/sdb/sdb.h>

#ifdef CONFIG_FTRACE
#include <elf.h>
extern char *ftrace_file;
extern Elf32_Ehdr *ftrace_file_header;
extern Elf32_Shdr *ftrace_file_sections;
extern char *ftrace_file_strtab;
extern Elf32_Sym *ftrace_file_symtab;
extern int ftrace_file_symtab_num;

#define CALL_FUNC_TIMES 20 // 定义调用函数的次数, 这里设置为20次, 也可以根据需要调整

// 获取与pc匹配的符号名称
static char *get_symbol_name(vaddr_t pc) {
    for (int i = 0; i < ftrace_file_symtab_num; i++) {
        // 因为st_info包含了符号绑定和类型信息，所以需要使用ELF32_ST_TYPE宏来获取符号类型
        if (ELF32_ST_TYPE(ftrace_file_symtab[i].st_info) == STT_FUNC && pc >= ftrace_file_symtab[i].st_value &&
            pc < ftrace_file_symtab[i].st_value + ftrace_file_symtab[i].st_size) {
            // 因为st_name是字符串在字符串表的偏移量, 所以要加上ftrace_file_strtab进行指针计算
            return ftrace_file_strtab + ftrace_file_symtab[i].st_name;
        }
    }
    return NULL; // 如果没有找到匹配的符号名称，则返回NULL
}

// 函数调用栈，用于跟踪调用和返回
static struct {
    vaddr_t pc;
    char name[64];
} call_stack[CALL_FUNC_TIMES];
static int call_stack_depth = 0;

// static void show_stack() {
//     // 打印调用栈
//     for (int i = 0; i < call_stack_depth; i++) {
//         printf("  %s at " FMT_WORD "\n", call_stack[i].name, call_stack[i].pc);
//     }
// }

// ftrace_elf_start函数用于跟踪程序中的函数调用和返回
static void ftrace_elf_start(vaddr_t pc, vaddr_t dnpc, bool is_call, bool is_ret) {
    if (ftrace_file == NULL || ftrace_file_header == NULL || ftrace_file_sections == NULL ||
        ftrace_file_strtab == NULL || ftrace_file_symtab == NULL) {
        return; // 如果跟踪文件或相关数据未初始化，则直接返回
    }

    if (is_call) {
        // 对于函数调用，查找跳转目标地址的符号
        char *target_symbol = get_symbol_name(dnpc);
        if (target_symbol != NULL) {
            // 检查是否为重复调用（避免无限递归的情况）
            bool is_duplicate = false;
            if (call_stack_depth > 0) {
                is_duplicate = (strcmp(call_stack[call_stack_depth - 1].name, target_symbol) == 0);
            }

            if (!is_duplicate) {
                // 打印缩进表示调用层次
                for (int i = 0; i < call_stack_depth; i++) {
                    printf("  ");
                }
                Log("call: %s at " FMT_WORD, target_symbol, dnpc);

                // 将函数信息压入调用栈
                if (call_stack_depth < CALL_FUNC_TIMES) {
                    call_stack[call_stack_depth].pc = dnpc;
                    strncpy(call_stack[call_stack_depth].name, target_symbol,
                            sizeof(call_stack[call_stack_depth].name) - 1);
                    call_stack[call_stack_depth].name[sizeof(call_stack[call_stack_depth].name) - 1] = '\0';
                    call_stack_depth++;
                } else {
                    Log("Warning: call stack overflow, max depth %d reached", CALL_FUNC_TIMES);
                }
            }
        }
    } else if (is_ret && call_stack_depth > 0) {
        // 对于函数返回，检查当前pc是否在栈顶函数的范围内
        char *current_symbol = get_symbol_name(pc);
        if (current_symbol != NULL && strcmp(call_stack[call_stack_depth - 1].name, current_symbol) == 0) {
            // 函数返回
            call_stack_depth--;
            for (int i = 0; i < call_stack_depth; i++) {
                printf("  ");
            }
            Log("return: %s at " FMT_WORD, current_symbol, pc);
        }
        // 如果当前符号不匹配，可能是从内联函数或其他情况返回，也尝试处理
        else if (call_stack_depth > 0) {
            call_stack_depth--;
            for (int i = 0; i < call_stack_depth; i++) {
                printf("  ");
            }
            Log("return: %s at " FMT_WORD, call_stack[call_stack_depth].name, pc);
        }
    }
}

#endif // CONFIG_FTRACE

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;     // g_nr_guest_inst用于记录cpu运行的指令数
static uint64_t g_timer = 0;      // unit: us    // g_timer用于记录cpu运行的时间，默认为0
static bool g_print_step = false; // g_print_step用于控制是否打印指令跟踪信息

char iringbuf[64][128] = {}; // iringbuf用于存储指令跟踪日志
int iringbuf_head = 0;       // iringbuf_head用于指示iring

#ifdef CONFIG_ITRACE
static void iringbuf_write(Decode *s) {
    if (s->logbuf[0] == '\0') {
        return; // 如果日志缓冲区为空，则不进行任何操作
    }
    if (iringbuf_head < sizeof(iringbuf) / sizeof(iringbuf[0])) {
        snprintf(iringbuf[iringbuf_head], sizeof(iringbuf[0]), "%s", s->logbuf);
        iringbuf_head++;
    } else {
        // 如果iringbuf已满，则覆盖最旧的记录
        iringbuf_head = 0;
        snprintf(iringbuf[iringbuf_head], sizeof(iringbuf[0]), "%s", s->logbuf);
    }
}
#endif

void device_update();

static void __attribute__((unused)) check_wp_update() {
    if (update_wp() > 0) {
        nemu_state.state = NEMU_STOP;
    }
}

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) {
        log_write("%s\n", _this->logbuf);
        IFDEF(CONFIG_ITRACE, iringbuf_write(_this));
    }
#endif
    /* 如果全局变量g_print_step被定义，输出跟踪的指令信息 */
    if (g_print_step) {
        IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
    }
    IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc)); // 步进调试
    IFDEF(CONFIG_WATCHPOINT, check_wp_update());            // 检查监视点是否被触发
}

/* 执行一次cpu的指令 */ // pc为实际执行指令地址, dnpc为下一次要执行的指令地址
// execute->exec_once->isa_exec_once->encode
static void exec_once(Decode *s, vaddr_t pc) {
    s->pc = pc;
    s->snpc = pc;
    isa_exec_once(s); // 执行指令,修改s->snpc,使s->snpc指向下一条指令的地址

#ifdef CONFIG_FTRACE
    // 检测函数调用和返回
    bool is_call = false;
    bool is_ret = false;

    // 获取指令
    uint32_t instruction = s->isa.inst;

    // 检测JAL指令 (无条件跳转并链接)
    if ((instruction & 0x7f) == 0x6f) { // JAL opcode
        uint32_t rd = (instruction >> 7) & 0x1f;
        // 任何跳转到函数地址的JAL指令都可能是函数调用
        char *target_symbol = get_symbol_name(s->dnpc);
        if (target_symbol != NULL && rd != 0) { // rd != 0 表示会保存返回地址
            is_call = true;
        }
    }
    // 检测JALR指令 (间接跳转并链接)
    else if ((instruction & 0x7f) == 0x67) { // JALR opcode
        uint32_t rd = (instruction >> 7) & 0x1f;
        uint32_t rs1 = (instruction >> 15) & 0x1f;

        // 函数调用模式：rd != 0 (保存返回地址) 且跳转到函数符号
        if (rd != 0) {
            char *target_symbol = get_symbol_name(s->dnpc);
            if (target_symbol != NULL) {
                is_call = true;
            }
        }
        // 函数返回模式：rd == 0 且 rs1 == 1 (返回地址寄存器)
        else if (rd == 0 && rs1 == 1) {
            is_ret = true;
        }
        // 其他返回模式：rd == 0 且从当前函数返回
        else if (rd == 0) {
            char *current_symbol = get_symbol_name(pc);
            if (current_symbol != NULL && call_stack_depth > 0 &&
                strcmp(call_stack[call_stack_depth - 1].name, current_symbol) == 0) {
                is_ret = true;
            }
        }
    }

    ftrace_elf_start(pc, s->dnpc, is_call, is_ret);
#endif

    cpu.pc = s->dnpc; // 更新pc
#ifdef CONFIG_ITRACE
    char *p = s->logbuf;
    p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
    int ilen = s->snpc - s->pc;
    int i;
    uint8_t *inst = (uint8_t *)&s->isa.inst;
#ifdef CONFIG_ISA_x86
    for (i = 0; i < ilen; i++) {
#else
    for (i = ilen - 1; i >= 0; i--) {
#endif
        p += snprintf(p, 4, " %02x", inst[i]);
    }
    int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
    int space_len = ilen_max - ilen;
    if (space_len < 0)
        space_len = 0;
    space_len = space_len * 3 + 1;
    memset(p, ' ', space_len);
    p += space_len;

    void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
    disassemble(p, s->logbuf + sizeof(s->logbuf) - p, MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst,
                ilen);
#endif
}

/* 实际运行cpu指令 n为运行的指令数 */
// engin_start->sdb_mainloop->other_orders->cpu_exec->execute
static void execute(uint64_t n) {
    Decode s;
    for (; n > 0; n--) {
        // 实际运行cpu指令
        exec_once(&s, cpu.pc);
        g_nr_guest_inst++;
        trace_and_difftest(&s, cpu.pc);
        if (nemu_state.state != NEMU_RUNNING) {
            if (nemu_state.state == NEMU_STOP) {
                // 若监视点触发则输出相关提示并退出cpu运行
                printf("Touched watchpoint\n");
            }
            break;
        }
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
    if (g_timer > 0)
        Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
    else
        Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
    isa_reg_display();
    statistic();
}

/* Simulate how the CPU works. */
/* 模拟cpu运行 获取的n为cpu运行的指令数 */
// engin_start->sdb_mainloop->other_orders->cpu_exec
void cpu_exec(uint64_t n) {
    /* 运行的指令数为负数时，表示一直运行到遇到异常或中断 */
    g_print_step = (n < MAX_INST_TO_PRINT);
    /* 判断nemu状态，如果处于未运行状态则停止对cpu的模拟 */
    switch (nemu_state.state) {
        case NEMU_END:
        case NEMU_ABORT:
        case NEMU_QUIT:
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

        case NEMU_END:
        case NEMU_ABORT:
            Log("nemu: %s at pc = " FMT_WORD,
                (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED)
                                                : (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN)
                                                                            : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
                nemu_state.halt_pc);
            // fall through
        case NEMU_QUIT: statistic();
    }
}
