/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <common.h>


#include "./monitor/sdb/sdb.h"

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
    /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
    am_init_monitor();
#else
    /* 将客户程序读入到客户计算机中 */
    init_monitor(argc, argv);
#endif
    // FILE *fp = fopen("/home/waysorry/ysyx/ysyx-workbench/nemu/tools/gen-expr/build/input", "r");
    // if (fp == NULL) {
    //     fprintf(stderr, "Failed to open input file.\n");
    //     return 1;
    // }
    // char *temp = (char *)malloc(1000 * sizeof(char));
    // while (fgets(temp, 1000, fp) != NULL) {
    //     char *temps = strtok(temp, " ");
    //     if (temps == NULL) {
    //         continue;
    //     }
    //     char *exprs = temps + strlen(temps) + 1;
    //     exprs[strlen(exprs) - 1] = '\0'; // Remove newline character at the end
    //     word_t result;
    //     sscanf(temps, "%u", &result);
    //     bool success = true;
    //     word_t res = expr(exprs, &success);
    //     if (!success) {
    //         printf("Error in expression: %s\n", exprs);
    //     } else if (res != result) {
    //         printf("Expression: %s, Expected: %u, Got: %u\n", exprs, result, res);
    //         return -1;
    //     } else {
    //         printf("Match\n");
    //     }
    //     // printf("Expression: %s, Expected: %u, Got: %u\n", exprs, result, res);
    // }
    // fclose(fp);
    // return 0;
    //
    /* Start engine. */
    engine_start();

    return is_exit_status_bad();
}
