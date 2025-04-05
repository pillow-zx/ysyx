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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

/* 以下为个人添加的头文件 */
#include <memory/paddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
/* 使用readline获取用户输入 */
static char* rl_gets() {
    static char* line_read = NULL;

    /* 检查line_read指针是否被占用 */
    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    /* 获取用户输入 */
    line_read = readline("(nemu) ");

    /* 如果用户输入不为空，则将其添加到历史记录中 */
    if (line_read && *line_read) {
        /* add_history函数在readline.h中定义，能够将line_read存入命令行历史 */
        add_history(line_read);
    }

    return line_read;
}

/* 模拟cpu运行 */
static int cmd_c(char* args) {
    cpu_exec(-1);
    return 0;
}

/* 退出 nemu */
static int cmd_q(char* args) {
    return -1;
}

static int cmd_help(char* args);

/* TODO: 单步调试功能 */
// 程序单步执行N条指令后暂停执行,当N没有给出时, 缺省为1
static int cmd_si(char* args) {
    int n;
    if (args == NULL) {
        n = 1;
    }
    else if ((n = atoi(args)) <= 0) {
        printf("Invalid argument: %s\n", args);
        return -1;
    }
    n = atoi(args);
    cpu_exec(n);
    return 0;
}

/* TODO: 显示寄存器信息 */
// "r"打印寄存器状态, "w"打印监视点信息
static int cmd_info(char* args) {
    if (args == NULL) {
        printf("Invalid argument: %s\n", args);
        return -1;
    }
    /* 打印寄存器状态信息 */
    if (strcmp(args, "r") == 0) {
        isa_reg_display();
    }
    /* 打印监视点信息 */
    else if (strcmp(args, "w") == 0) {
        /* 我把打印监视点的函数放到watchpoint.c下，同时在sdb.h引用他 */
        display_wp();
    }
    else {
        printf("Unknown command '%s'\n", args);
    }
    return 0;
}

/* TODO: 求出指定表达式的值将结果作为起始内存，地址, 以十六进制形式输出连续的指定个4字节*/
static int cmd_x(char* args) {
    if (args == NULL) {
        printf("Invalid argument: %s\n", args);
        return -1;
    }
    char* temp[2] = { NULL, NULL };
    /* 获取第一个子字符串 */
    char* token = strtok(args, " ");
    temp[0] = token;
    /* 继续获取其他的子字符串 */
    token = strtok(NULL, " ");
    temp[1] = token;
    int num[2];
    /* 将第一个子字符串转换为整数 */
    num[0] = atoi(temp[0]);
    /* 将第二个子字符串转换为整数 */
    if (temp[1] == NULL) {
        printf("Invalid argument: %s\n", args);
        return -1;
    }
    /* 将第二个子字符串转换为整数 */
    num[1] = atoi(temp[1] + 2);
    char* endptr;
    int addr = (int)strtol(temp[1], &endptr, 16);
    /* 打印内存 */
    for (int i = num[1]; i < num[0] * 4 + num[1]; i += 4) {
        /* 读取内存 */
        uint32_t data = paddr_read(addr + i, 4);
        /* 打印内存 */
        printf("0x%08x: 0x%08x\n", addr + i, data);
    }
    return 0;
}

/* 内置指令 */
static struct {
    const char* name;
    const char* description;
    int (*handler) (char*);
} cmd_table[] = {
    /*         指令名                       描述                                              运行的函数*/
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  /* 在此处添加更多指令 */
  { "si", "Step into the program", cmd_si },
  { "info", "Show information about registers", cmd_info },
  { "x", "Examine memory", cmd_x },
};

#define NR_CMD ARRLEN(cmd_table)    // 计算cmd_table的长度

static int cmd_help(char* args) {
    /* extract the first argument */
    /* strtok函数将args分割成多个子串，返回第一个子串的指针 */
    char* arg = strtok(NULL, " ");
    int i;

    /* 如果没有参数，则打印所有命令的帮助信息 */
    if (arg == NULL) {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++) {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    }
    /* 如果有参数，则打印该命令的帮助信息 */
    else {
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
                return 0;
            }
        }
        /* 如果没有找到该命令，则打印错误信息 */
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void sdb_set_batch_mode() {
    is_batch_mode = true;
}

void sdb_mainloop() {
    /* 是否打开批处理模式 */
    if (is_batch_mode) {
        cmd_c(NULL);
        return;
    }

    /* 获取用户输入 */
    for (char* str; (str = rl_gets()) != NULL; ) {
        /* 将str_end指向的位置从str指向的位置开始，向后移动strlen(str)位, 即将str_end指向str末尾(\0) */
        char* str_end = str + strlen(str);
        // 这是一个典型的c语言指针运算，+号表示指针向后移动，-号表示指针向前移动

        /* extract the first token as the command */
        /* strtok函数将str分割成多个子串，返回第一个子串的指针 */
        char* cmd = strtok(str, " ");
        /* 若未获得输入则跳过此次循环 */
        if (cmd == NULL) { continue; }

        /* treat the remaining string as the arguments,
         * which may need further parsing
         */
         /* 正常情况下args应该指向命令后第一个字符位置，在此处一般为"\0" */
        char* args = cmd + strlen(cmd) + 1;
        /* 如果args >= str_end则没有参数, 因为数字的ascii码一定比字母小 */
        if (args >= str_end) {
            args = NULL;
        }

#ifdef CONFIG_DEVICE
        extern void sdl_clear_event_queue();
        sdl_clear_event_queue();
#endif

        int i;
        /* 找到内置指令 */
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                /* 判断是否推出nemu */
                if (cmd_table[i].handler(args) < 0) { return; }
                break;
            }
        }
        /* 如果没有找到内置指令，则打印错误信息 */
        if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
    }
}

void init_sdb() {
    /* Compile the regular expressions. */
    /* 初始化正则表达式 */
    init_regex();

    /* Initialize the watchpoint pool. */
    /* 初始化工作池。该函数不接受任何参数且不返回任何值 */
    init_wp_pool();
}
