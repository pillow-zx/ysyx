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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>
#include <string.h>


typedef uint32_t word_t;

// 缓冲区大小应该足够
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // 比 buf 稍大一些
static char *code_format = "#include <stdio.h>\n"
                           "int main() { "
                           "  unsigned result = %s; "
                           "  printf(\"%%u\", result); "
                           "  return 0; "
                           "}";


// 跟踪字符串位置
static word_t pos = 0;

// 添加最大递归深度限制
#define MAX_DEPTH 4
static word_t current_depth = 0;

// 生成 [0, n-1] 范围内的随机数
static word_t choose(unsigned n) {
    return rand() % n;
}

// 在缓冲区中添加单个字符
static void gen(char ch) {
    buf[pos++] = ch;
    buf[pos] = '\0';
}

// 生成随机数字（1-100范围内）
static void gen_num() {
    // 生成1到100之间的数字，避免0
    word_t num = 1 + choose(100);
    pos += sprintf(buf + pos, "%d", num);
}
// 前向声明
static void gen_rand_expr(void);

// 生成安全的除数（永远不会为0）
static void gen_safe_divisor() {
    buf[pos] = '\0';
    // 生成一个正数作为除数
    gen('(');
    gen_num();
    gen(')');
}

// 生成随机运算符
static void gen_rand_op_expr() {
    // 可用运算符: +, -, *, /
    char ops[] = {'+', '-', '*', '/'};
    char op = ops[choose(4)];

    // 如果是除法，确保除数非零
    if (op == '/') {
        gen(' ');
        gen('/');
        gen(' ');
        gen_safe_divisor();
    } else {
        gen(' ');
        gen(op);
        gen(' ');
        gen_rand_expr();
    }
}

// 生成随机表达式
static void gen_rand_expr() {
    // 如果达到最大深度，只生成数字
    if (current_depth >= MAX_DEPTH) {
        gen_num();
        return;
    }

    // 增加当前深度
    current_depth++;

    // 根据当前深度调整生成策略
    int choice;
    if (current_depth >= MAX_DEPTH - 1) {
        // 在接近最大深度时，增加生成单个数字的概率
        choice = choose(2) ? 0 : 2;
    } else {
        choice = choose(3);
    }

    switch (choice) {
        case 0: // 生成单个数字
            gen_num();
            break;

        case 1: // 生成带括号的表达式
            gen('(');
            gen_rand_expr();
            gen(')');
            break;

        default: // 生成二元运算
            gen('(');
            gen_rand_expr();
            gen_rand_op_expr();
            gen(')');
            break;
    }

    // 减少当前深度
    current_depth--;
}

int main(int argc, char *argv[]) {
    int seed = time(0);
    srand(seed);
    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }
    int i;
    for (i = 0; i < loop; i++) {
        pos = 0;           // 重置位置以生成新表达式
        current_depth = 0; // 重置递归深度
        gen_rand_expr();

        sprintf(code_buf, code_format, buf);

        FILE *fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
        if (ret != 0)
            continue;

        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        int result;
        ret = fscanf(fp, "%d", &result);
        pclose(fp);

        printf("%u %s\n", result, buf);
    }
    return 0;
}
