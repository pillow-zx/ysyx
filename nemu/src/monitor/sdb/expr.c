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

#include "common.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdio.h>
#include <string.h>

 // 个人添加头文件
#include <stdbool.h>

enum {
    // 256            257
    TK_NOTYPE = 256, TK_EQ,

    /* TODO: Add more token types */

};


/* 正则表达式规则表 */
static struct rule {
    const char* regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},    // spaces 256
    {"\\+", '+'},         // plus   43
    {"==", TK_EQ},        // equal  257

    /* 个人添加的rules */
    { "\\-", '-' },         // minus   45
    { "\\*", '*' },         // star    42
    { "/", '/' },           // slash   47
    { "\\(", '(' },         // lparent 40
    { "\\)", ')' },         // rparent 41
    { "[0-9]+", '0' },      // number  0
};

/* 返回rules的大小 */
#define NR_REGEX ARRLEN(rules)

/* regex_t 是 POSIX 标准中定义的正则表达式结构体类型，用于存储编译后的正则表达式信息 */
/* 创建一个regex_t类型的结构体数组用于存储匹配到的正则表达式模式 */
static regex_t re[NR_REGEX] = {};   // 该数组长度等于rules的个数

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */

 /* 初始化正则表达式 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    /* 逐个检查 */
    for (i = 0; i < NR_REGEX; i++) {
        // REG_EXTENDED 是一个宏定义，其值为 1。如果设置了这个位，则使用扩展正则表达式语法；如果未设置，则使用基本正则表达式语法
        /* regcomp 函数用于编译正则表达式，并将结果存储在 regex_t 类型的结构中，以便在后续的正则表达式匹配操作中使用。
        *  该函数遵循 POSIX 标准，参数包括目标结构指针 __preg、正则表达式模式字符串 __pattern 和编译标志 __cflags
        */
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        // 如果编译成功，ret 的值为 0；如果编译失败，ret 的值为非零值
        if (ret != 0) {
            /* regerror 函数用于将正则表达式库函数返回的错误代码转换为人类可读的错误消息。
            *  它接受错误代码、正则表达式对象、用于存储错误消息的缓冲区及其大小，并返回错误消息的长度。
            */
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

typedef struct token {
    int type;       // 记录token类型，即rules中的token_type
    char str[32];   // 记录token字符串
} Token;

/* __attribute__((used)): 是GCC特有的属性声明
* 作用：告诉编译器即使该变量看起来未被使用，也要保留它
* 防止编译器优化时删除这个看似未使用的变量
*/
static Token tokens[32] __attribute__((used)) = {}; // tokens数组用于按顺序存放已经被识别出的token信息
static int nr_token __attribute__((used)) = 0;      // nr_token指示已经被识别出的token数目

// 用position变量来指示当前处理到的位置, 并且按顺序尝试用不同的规则来匹配当前位置的字符串. 
// 当一条规则匹配成功, 并且匹配出的子串正好是position所在位置的时候, 我们就成功地识别出一个token, Log()宏会输出识别成功的信息. 
static bool make_token(char* e) {
    int position = 0;
    int i;
    /* regmatch_t 是 POSIX 标准中定义的结构体类型，用于存储正则表达式匹配结果的信息 */
    /* regmatch_t 结构体包含两个int类型成员 rm_so 和 rm_eo，分别表示匹配到的子串在原字符串中的起始和结束位置 */
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        /* 逐个检查正则表达式规则 */
        for (i = 0; i < NR_REGEX; i++) {
            // regexec 函数用于在 C 语言中执行正则表达式匹配操作。regexec 函数的变量含义为：
            // 指向已编译的正则表达式的指针 要匹配的字符串 可存储的最大匹配数 存储匹配结果的数组  匹配标志
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char* substr_start = e + position;  // substr_start 是一个指向字符的指针，用于在 C 语言中执行正则表达式匹配操作时，指向匹配结果的起始位置。
                int substr_len = pmatch.rm_eo;      // substr_len 是一个整数变量，用于存储子字符串的长度。

                // %.*s 是 printf 格式化字符串中的一个特殊格式说明符，用于打印指定长度的字符串。
                // % - 格式说明符的开始， .* - 精度由参数指定， s - 字符串类型
                // %.*s 会打印字符串的前 n 个字符，其中 n 由 substr_len 指定
                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len, substr_start);

                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                 /* 现在一个新的token已经被被识别为rules[i]。添加代码来记录token在数组tokens中。
                  * 对于某些类型的token，应该执行一些额外的操作。
                  */
                if (nr_token >= 32) {
                    printf("Too many tokens\n");
                    return false;
                }
                tokens[nr_token].type = rules[i].token_type;
                strncpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = '\0'; // 添加字符串结束符

                switch (rules[i].token_type) {
                case TK_NOTYPE: // 忽略空格
                    break;
                case TK_EQ: // 等于
                    nr_token++;
                    break;
                case '0': // 数字
                    // 将字符串转换为整数
                    nr_token++;
                    break;
                case '+': case '-': case '*': case '/':
                    // 运算符只需记录类型，无需额外处理 
                    nr_token++;
                    break;
                case '(': case ')':
                    // 括号只需记录类型，无需额外处理
                    nr_token++;
                    break;
                default:
                    // 其他类型的token可以在这里进行处理
                    printf("Unknown token type %d at position %d\n%s\n%*.s^\n",
                        rules[i].token_type, position, e, position, "");
                    break;
                }

                break;
            }
        }

        /* 当已经找到最后仍然没有找到时显示 */
        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }

    return true;
}

// 个人添加的辅助函数

#define MAX_PRIORITY 100

// 定义一个栈结构体，用于存储操作数和操作符
typedef struct {
    word_t value[MAX_PRIORITY];
    int top;
} Stack;

typedef struct {
    char op[MAX_PRIORITY];
    int top;
} OpStack;


// 初始化栈
void init_stack(Stack* num, OpStack* op) {
    num->top = -1;
    op->top = -1;
}

// 判断是否为运算符
int is_operator(char op) {
    return (op == '+' || op == '-' || op == '*' || op == '/');
}

// 判断运算符优先级
int precedence(char op) {
    if (op == '+' || op == '-') { return 1; }
    if (op == '*' || op == '/') { return 2; }
    return 0;
}

// 将操作数压入栈
void apply_operator(Stack* num, char op) {
    word_t b = num->value[num->top--];
    word_t a = num->value[num->top--];

    switch (op) {
    case '+': num->value[++num->top] = a + b; break;
    case '-': num->value[++num->top] = a - b; break;
    case '*': num->value[++num->top] = a * b; break;
    case '/':
        if (b == 0) {
            printf("Error: Division by zero\n");
            return;
        }
        num->value[++num->top] = a / b;
        break;
    }
}

#include <ctype.h>

word_t calculate_expression() {
    Stack num;
    OpStack op;
    init_stack(&num, &op);

    for (int i = 0; tokens[i].str[0]; i++) {
        // 跳过空格
        if (isspace(tokens[i].str[0])) { continue; }

        if (isdigit(tokens[i].str[0])) {
            // 如果是数字，将其压入操作数栈
            word_t value = 0;
            while (isdigit(tokens[i].str[0])) {
                value = value * 10 + (tokens[i].str[0] - '0');
                i++;
            }
            i--;
            num.value[++num.top] = value;
        }
        else if (tokens[i].str[0] == '(') {
            // 如果是左括号，将其压入操作符栈
            op.op[++op.top] = tokens[i].str[0];
        }
        else if (tokens[i].str[0] == ')') {
            // 如果是右括号，弹出操作符栈直到遇到左括号
            while (op.top >= 0 && op.op[op.top] != '(') {
                apply_operator(&num, op.op[op.top--]);
            }
            op.top--; // 弹出左括号
        }
        else if (is_operator(tokens[i].str[0])) {
            // 如果是运算符
            while (op.top >= 0 && precedence(op.op[op.top]) >= precedence(tokens[i].str[0])) {
                apply_operator(&num, op.op[op.top--]);
            }
            op.op[++op.top] = tokens[i].str[0]; // 压入操作符栈
        }
    }
    while (op.top >= 0) {
        apply_operator(&num, op.op[op.top--]);
    }
    return num.value[0]; // 返回栈顶的值，即表达式的结果
}



// word_t 为unsigned int类型, 32位
word_t expr(char* e, bool* success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    /* 这里可以添加代码来计算表达式的值 */
    word_t result = calculate_expression();
    return result;
}

/* This function will be used in expression evaluation */