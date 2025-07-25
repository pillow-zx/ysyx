// #include "tools.h"
// #include "cpu.h"

// // TODO: Implement the expression evaluation logic

// std::vector<std::string> rules = {
//     "0x[0-9a-fA-F]+",         // Hexadecimal numbers  16进制数
//     "[0-9]+",                 // Decimal numbers      十进制数
//     "[a-zA-Z_][a-zA-Z0-9_]*", // Identifiers (variables, registers) 标识符（变量、寄存器）
//     "\\+",                    // Addition               // 加法
//     "-",                      // Subtraction            // 减法
//     "\\*",                    // Multiplication         // 乘法
//     "/",                      // Division               // 除法
//     "%",                      // Modulus                // 取模
//     "\\(",                    // Left parenthesis       // 左括号
//     "\\)",                    // Right parenthesis      // 右括号
// };

// typedef struct token {
//     std::string type; // Token type (e.g., number, identifier, operator)
//     std::string value; // Token value
// } Token;

// static Token tokens[100]; // Token array to hold parsed tokens
// static int token_count = 0; // Number of tokens parsed

// static void get_tokens(const std::string &expression) {
//     size_t pos = 0;
//     while (pos < expression.length()) {
//         bool matched = false;
//         for (const auto &rule : rules) {
//             std::regex re(rule);
//             std::smatch match;
//             if (std::regex_search(expression.begin() + pos, expression.end(), match, re)) {
//                 if (match.position() == 0) {
//                     tokens[token_count].type = rule;
//                     tokens[token_count].value = match.str();
//                     token_count++;
//                     pos += match.length();
//                     matched = true;
//                     break;
//                 }
//             }
//         }
//         if (!matched) {
//             pos++;
//         }
//     }
// }

// uint32_t expr(const std::string &expression) {
//     get_tokens(expression);
//     return 0;
// }