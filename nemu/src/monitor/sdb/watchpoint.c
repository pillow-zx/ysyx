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

#include "sdb.h"

#define NR_WP 32

/* 定义监视点结构体 */
typedef struct watchpoint {
    int NO;
    struct watchpoint* next;

    /* TODO: Add more members if necessary */

} WP; // 监视点结构体

static WP wp_pool[NR_WP] = {};  // 监视点池，最多有32个监视点
static WP* head = NULL, * free_ = NULL; // 监视点链表头指针和空闲链表头指针

/* 初始化工作池 */
void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    }

    head = NULL;
    free_ = wp_pool;
}

WP* new_wp() {
    if (free_ == NULL) {
        printf("No free watchpoint!\n");
        assert(0);
    }
    WP* wp = free_;
    free_ = free_->next;
    wp->next = head;
    head = wp;
    return wp;
}

void free_wp(WP* wp) {

}

/* TODO: Implement the functionality of watchpoint */
void display_wp() {
    head = free_;
    while (head) {
        printf("Watchpoint %d\n", head->NO);
        head = head->next;
    }
}


