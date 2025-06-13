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

#include "common.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>

enum {
    TK_NOTYPE = 256,
    TK_EQ,

    /* TODO: Add more token types */
    TK_16NUM,
    TK_10NUM,
    TK_AND,
    TK_N_EQ,
    TK_REGS,
    TK_POINT,
    TK_NEGTIVE,
};

static struct rule {
    const char *regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

    {" +", TK_NOTYPE},               // spaces
    {"\\+", '+'},                    // plus
    {"==", TK_EQ},                   // equal
    {"-", '-'},                      // minus
    {"\\*", '*'},                    // multiply
    {"/", '/'},                      // divide
    {"\\(", '('},                    // left parenthesis
    {"\\)", ')'},                    // right parenthesis
    {"0[xX][0-9a-fA-F]+", TK_16NUM}, // hex number
    {"[0-9]+", TK_10NUM},            // decimal number
    {"&&", TK_AND},
    {"!=", TK_N_EQ},
    {"\\$(\\%0|ra|sp|gp|tp|t[0-6]|s[0-9]|s10|s11|a[0-7])", TK_REGS},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
// 初始化正则表达式
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

// 定义token结构体
typedef struct token {
    int type;
    char str[32];
} Token;

static Token tokens[10000] __attribute__((used)) = {};
// static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

// 从字符串e中提取token
static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        // 逐个尝试所有规则
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                // 获取匹配的字符串
                char *substr_start = e + position; // 匹配字符串起始位置
                int substr_len = pmatch.rm_eo;     // 匹配字符串长度

                // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex,
                //    position, substr_len, substr_len, substr_start);

                position += substr_len; // 更新指针位置

                /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

                switch (rules[i].token_type) {
                    // 空格不计入
                    case TK_NOTYPE:
                        break;
                    // 加减乘除法和逻辑运算直接放入token
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                    case '(':
                    case ')':
                    case TK_EQ:
                    case TK_AND:
                    case TK_N_EQ:
                        Assert(nr_token < 10000, "Too many tokens");
                        tokens[nr_token].type = rules[i].token_type;

                        // 判断是否为负号或解引用
                        if (tokens[nr_token].type == '*' || tokens[nr_token].type == '/') {
                            if (tokens[nr_token - 1].type == 0 ||
                                tokens[nr_token - 1].type == '(' ||
                                tokens[nr_token - 1].type == '+' ||
                                tokens[nr_token - 1].type == '-' ||
                                tokens[nr_token - 1].type == TK_EQ ||
                                tokens[nr_token - 1].type == TK_AND ||
                                tokens[nr_token - 1].type == TK_N_EQ) {
                                tokens[nr_token].type =
                                    tokens[nr_token].type == '*' ? TK_POINT : TK_NEGTIVE;
                            }
                        }

                        nr_token++;
                        break;
                    // 读取十六进制和十进制数字
                    case TK_10NUM:
                        Assert(substr_len < 10000, "Too long number");
                        strncpy(tokens[nr_token].str, substr_start,
                                substr_len);                     // 将匹配的字符串复制到token中
                        tokens[nr_token].str[substr_len] = '\0'; // 添加'\0'结尾
                        tokens[nr_token].type = TK_10NUM;        // 设置token类型为十进制数字
                        nr_token++;
                        break;
                    case TK_16NUM:
                        Assert(substr_len < 10000, "Too long number");
                        strncpy(tokens[nr_token].str, substr_start, substr_len);
                        tokens[nr_token].str[substr_len] = '\0';
                        tokens[nr_token].type = TK_16NUM;
                        nr_token++; // 设置token类型为十六进制数字
                        break;
                        // 获取寄存器的值
                    case TK_REGS:
                        Assert(substr_len < 10000, "Too long register");
                        strncpy(tokens[nr_token].str, substr_start, substr_len);
                        tokens[nr_token].str[substr_len] = '\0';
                        tokens[nr_token].type = TK_REGS; // 设置token类型为寄存器
                        nr_token++;
                        break;
                    default:
                        Assert(false, "Unknown token type");
                }
                break;
            }
        }

        // 未匹配到任何规则
        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }

    return true;
}

// 检查括号是否匹配
bool check_parentheses(int p, int q) {
    if (tokens[p].type != '(' || tokens[q].type != ')') {
        return false;
    }
    int parenthesis_count = 0;
    for (int i = p + 1; i < q - 1; i++) {
        if (tokens[i].type == '(') {
            parenthesis_count++;
        } else if (tokens[i].type == ')') {
            parenthesis_count--;
        }
        if (parenthesis_count < 0) {
            return false;
        }
    }
    return true;
}

// 规定运算符的优先级
static int priority(int operator) {
    switch (operator) {
        case TK_AND:
            return 0;
        case TK_EQ:
        case TK_N_EQ:
            return 1;
        case '+':
        case '-':
            return 2;
        case '*':
        case '/':
            return 3;
        case TK_POINT:
        case TK_NEGTIVE:
            return 4;
        default:
            return 5;
    }
}

// 查找主运算符的位置
static int main_operator(int p, int q) {
    int position = -1;
    int main_operator = -1;
    int parenthesis_count = 0;
    for (int i = p; i <= q; i++) {
        int operator= tokens[i].type;
        switch (operator) {
            case '(':
                parenthesis_count++;
                break;
            case ')':
                parenthesis_count--;
                break;
            case '+':
            case '-':
            case '*':
            case '/':
            case TK_EQ:
            case TK_N_EQ:
            case TK_AND:
            case TK_POINT:
            case TK_NEGTIVE:
                if (parenthesis_count == 0 && priority(operator) <= priority(main_operator)) {
                    main_operator = operator;
                    position = i;
                }
                break;
            default:
                break;
        }
    }
    return position;
}

// 计算表达式的值
int32_t eval(int p, int q) {
    if (p > q) {
        Assert(false, "p > q");
    } else if (p == q) { // 输入只有一个token
        word_t result = 0;
        switch (tokens[p].type) {
            case TK_10NUM:
                sscanf(tokens[p].str, "%u", &result);
                return result;
                break;
            case TK_16NUM:
                sscanf(tokens[p].str, "%x", &result);
                return result;
                break;
            case TK_REGS:
                bool success = false;
                return isa_reg_str2val(tokens[p].str + 1, &success);
            default:
                Assert(false, "Unknown token type");
        }
    } else if (check_parentheses(p, q) == true) { // 检查括号是否匹配
        return eval(p + 1, q - 1);                // 去除括号
    } else {
        int position = main_operator(p, q); // 寻找主运算符的位置
        if (position == -1) {
            Assert(false, "No main operator");
        }

        // word_t val1 = eval(p, position - 1);
        // word_t val2 = eval(position + 1, q);

        int32_t val1 = eval(position + 1, q);              // 计算右侧表达式的值
        if (tokens[position].type == TK_POINT) {          // 指针解引用
            return vaddr_read(val1, 4);                   // 32位系统采用4位
        } else if (tokens[position].type == TK_NEGTIVE) { // 如果是负号
            return -val1;
        }
        int32_t val2 = eval(p, position - 1); // 计算左侧表达式的值
        // 执行运算
        switch (tokens[position].type) {
            case TK_EQ:
                return val2 == val1;
            case TK_AND:
                return val2 && val1;
            case TK_N_EQ:
                return val2 != val1;
            case '+':
                return val2 + val1;
            case '-':
                return val2 - val1;
            case '*':
                return val2 * val1;
            case '/':
                // 防止除以零
                if (val1 == 0) {
                    Assert(false, "Division by zero");
                }
                return val2 / val1;
            default:
                Assert(false, "Unknown operator");
        }
    }
    return 0;
}

word_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    // printf("Expression %s\n", e);
    /* TODO: Insert codes to evaluate the expression. */
    word_t result = eval(0, nr_token - 1);
    return result;
}
