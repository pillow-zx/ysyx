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
#include <ctype.h>

enum {
    TK_NOTYPE = 256,
    TK_EQ,

    /* TODO: Add more token types */

};

static struct rule {
    const char *regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces 空格
    {"\\+", '+'},      // plus 加法
    {"==", TK_EQ},     // equal 等于

    {"\\-", '-'},               // minus 减法
    {"\\*", '*'},               // multiply 乘法
    {"/", '/'},                 // divide 除法
    {"\\(", '('},               // left parenthesis 左括号
    {"\\)", ')'},               // right parenthesis 右括号
    {"0[xX][0-9a-fA-F]+", '0'}, // hex number 十六进制数
    {"[0-9]+", '0'},            // decimal number 十进制数
    {"\\$", '$'},               // ask reg 访问寄存器
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {}; // 获取正则表达式的数组大小

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
/* 初始化正则表达式 */
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

typedef struct token {
    int type;     // 用来记录token类型
    char str[32]; // 记录token字符串
} Token;

/* __attribute__ 告诉编译器即使该变量看起来未被使用，也要保留它
 * 防止编译器优化时删除这个看似未使用的变量
 */
static Token tokens[10000] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
    int position = 0; // 表示当前字符串的位置
    int i;
    regmatch_t pmatch;

    nr_token = 0; // 表示当前token的数量

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        // 挨个尝试所有规则
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                // 圈出匹配的字符串
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex,
                    position, substr_len, substr_len, substr_start);

                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                * to record the token in the array `tokens'. For certain types
                * of tokens, some extra actions should be performed.
                */

                if (substr_len > 32) {
                    printf("token string is too long: %.*s\n", substr_len, substr_start);
                    return false;
                } else if (rules[i].token_type == TK_NOTYPE) {
                    break; // ignore this token
                }
                tokens[nr_token].type = rules[i].token_type;
                strncpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = '\0'; // 末尾加上'\0'

                switch (rules[i].token_type) {
                case ' ':
                    continue; // ignore spaces
                case '0':
                case '+':
                case '-':
                case '*':
                case '/':
                case '(':
                case ')':
                    nr_token++;
                    break;
                default:
                    printf("unknown token type: %d\n", rules[i].token_type);
                    break;
                }
                break;
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }

    return true;
}

#define MAX_PRIORITY 1000

typedef struct {
    word_t value[MAX_PRIORITY];
    int top;
} Stack;

typedef struct {
    char op[MAX_PRIORITY];
    int top;
} OpStack;

void init_stack(Stack *s, OpStack *op) {
    s->top = -1;
    op->top = -1;
}

bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

int precedence(char c) {
    switch (c) {
    case '+':
    case '-':
        return 1;
    case '*':
    case '/':
        return 2;
    default:
        return 0;
    }
}

void apply_operator(Stack *s, OpStack *op) {
    word_t b;
    word_t a;

    switch (op->op[op->top--]) {
    case '+':
        b = s->value[s->top--];
        a = s->value[s->top--];
        s->value[++s->top] = a + b;
        break;
    case '-':
        b = s->value[s->top--];
        a = s->value[s->top--];
        s->value[++s->top] = a - b;
        break;
    case '*':
        b = s->value[s->top--];
        a = s->value[s->top--];
        s->value[++s->top] = a * b;
        break;
    case '/':
        b = s->value[s->top--];
        a = s->value[s->top--];
        if (b == 0) {
            printf("division by zero\n");
            return;
        }
        s->value[++s->top] = a / b;
        break;
    default:
        printf("unknown operator: %c\n", op->op[op->top]);
        return;
    }
}

word_t calculate_expression() {
    Stack s;
    OpStack op;
    init_stack(&s, &op);

    for (int i = 0; tokens[i].str[0]; i++) {
        // if (tokens[i].type == '0') {
        //     word_t val = 0;
        //     if (tokens[i].str[0] == '0' && (tokens[i].str[1] == 'x' || tokens[i].str[1] == 'X')) {
        //         for (int j = 2; tokens[i].str[j] != '\0'; j++) {
        //             val = val * 16 + (isdigit(tokens[i].str[j]) ? tokens[i].str[j] - '0' : tolower(tokens[i].str[j]) - 'a' + 10);
        //         }
        //     } else {
        //         for (int j = 0; tokens[i].str[j] != '\0'; j++) {
        //             val = val * 10 + (tokens[i].str[j] - '0');
        //         }
        //     }
        if (isdigit(tokens[i].str[0])) {
            unsigned int val = 0;
            sscanf(tokens[i].str, "%u", &val);
            s.value[++s.top] = val;
        } else if (tokens[i].str[0] == '(') {
            op.op[++op.top] = tokens[i].str[0];
        } else if (tokens[i].str[0] == ')') {
            while (op.op >= 0 && op.op[op.top] != '(') {
                apply_operator(&s, &op);
            }
            op.top--;
        } else if (is_operator(tokens[i].str[0])) {
            while (op.op >= 0 && precedence(op.op[op.top]) >= precedence(tokens[i].str[0])) {
                apply_operator(&s, &op);
            }
            op.op[++op.top] = tokens[i].str[0];
        }
    }
    while (op.top >= 0) {
        apply_operator(&s, &op);
    }
    return s.value[s.top];
}

word_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    return calculate_expression();
}
