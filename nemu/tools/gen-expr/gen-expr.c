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
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char* code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

char* buff_ptr = buf;

static unsigned choose(int n) {
    return rand() % n;
}

static void gen(char ch) {
    *buff_ptr++ = ch;
}

static void gen_num() {
    int num = choose(100); // 生成0-99的随机数
    char num_str[32];
    sprintf(num_str, "%d", num);
    int len = strlen(num_str);
    for (int i = 0; i < len; i++) {
        gen(num_str[i]);
    }
}

static void gen_rand_op() {
    char  ops[] = { '+', '-', '*', '/' };
    gen(ops[choose(4)]);
}

static void gen_rand_expr() {
    if (buff_ptr - buf > 65000) {
        return;
    }
    switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
    }
}

int main(int argc, char* argv[]) {
    /* 初始化随机数种子 */
    int seed = time(0);
    srand(seed);
    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }

    /* 执行loop次循环 */
    int i;
    for (i = 0; i < loop; i++) {
        buff_ptr = buf; // 重置buf_ptr
        memset(buf, 0, sizeof(buf)); // 清空buf
        gen_rand_expr();    // 生成随机表达式
        *buff_ptr = '\0';      // 添加字符串结束符

        /* 将buf中的表达式格式化到code_buf中 */
        sprintf(code_buf, code_format, buf);

        /* 将code_buf中的代码写入临时文件中 */
        FILE* fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        /* 编译临时文件中的代码 */
        int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
        if (ret != 0) continue;

        /* 执行编译后的代码 */
        /* 读取执行结果 */
        // file:///home/waysorry/user/NemuNote/function/pepen和pclose.md */
        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        /* 读取输出结果 */
        int result;
        ret = fscanf(fp, "%d", &result);
        pclose(fp);

        printf("%u %s\n", result, buf);
    }
    return 0;
}
