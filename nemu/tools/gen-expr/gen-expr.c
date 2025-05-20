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
static char *code_format = "#include <stdio.h>\n"
                           "int main() { "
                           "  unsigned result = %s; "
                           "  printf(\"%%u\", result); "
                           "  return 0; "
                           "}";

#define MAX_NUMBER 1000
static int buf_len = 0;

static uint32_t choose(uint32_t n) {
    return rand() % n;
}

// static void space() {
//     int n = rand() % 10;
//     for (int i = 0; i < n; i++) {
//         buf[buf_len++] = ' ';
//     }
// }

static void gen_num() {
    char num_str[32];
    uint32_t num;
    int i = 1;
    while (buf[buf_len - i] == ' ' && buf_len - i > 0) {
        i++;
    }
    if (buf_len > 0 && buf[buf_len - i] == '/') {
        num = choose(MAX_NUMBER) + 1; 
    } else {
        num = choose(MAX_NUMBER);
    }
    sprintf(num_str, "%u", num);
    int len = strlen(num_str);
    for (int i = 0; i < len; i++) {
        buf[buf_len++] = num_str[i];
    }
    // space();
}

static void gen(char ch) {
    buf[buf_len++] = ch;
    // space();
}

static void gen_rand_op() {
    char ops[] = {'+', '-', '*', '/'};
    buf[buf_len++] = ops[choose(4)];
    // space();
}

static void gen_rand_expr() {
    gen('(');
    switch (choose(3)) {
    case 0:
        gen_num();
        break;
    case 1:
        gen('(');
        gen_rand_expr();
        gen(')');
        break;
    default:
        gen_rand_expr();
        gen_rand_op();
        gen_rand_expr();
        break;
    }
    gen(')');
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
        gen_rand_expr();
        buf[buf_len] = '\0';

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
        buf_len = 0;
    }
    return 0;
}
