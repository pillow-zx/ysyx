#pragma once

#include <boost/format.hpp>

// Define default pc start
#define DEFAULT_PC_START 0x80000000

// Define NPC bits
#define NPC_BITS           32

// ANSI颜色代码（粗体版本）
#define ANSI_COLOR_RED     "\x1b[1;31m" // 红色
#define ANSI_COLOR_GREEN   "\x1b[1;32m" // 绿色
#define ANSI_COLOR_YELLOW  "\x1b[1;33m" // 黄色
#define ANSI_COLOR_BLUE    "\x1b[1;34m" // 蓝色
#define ANSI_COLOR_MAGENTA "\x1b[1;35m" // 品红色
#define ANSI_COLOR_CYAN    "\x1b[1;36m" // 青色
#define ANSI_COLOR_WHITE   "\x1b[1;37m" // 白色
#define ANSI_COLOR_RESET   "\x1b[0m"    // 重置颜色

// 背景颜色代码
#define ANSI_BG_RED        "\x1b[41m"
#define ANSI_BG_GREEN      "\x1b[42m"
#define ANSI_BG_YELLOW     "\x1b[43m"
#define ANSI_BG_BLUE       "\x1b[44m"

// 样式代码
#define ANSI_BOLD          "\x1b[1m"
#define ANSI_UNDERLINE     "\x1b[4m"

// 基础彩色输出宏（无参数版本）
#define LOG_INFO_0(msg)    std::cout << ANSI_COLOR_GREEN << "[INFO] " << ANSI_COLOR_RESET << msg << std::endl
#define LOG_WARN_0(msg)    std::cout << ANSI_COLOR_YELLOW << "[WARN] " << ANSI_COLOR_RESET << msg << std::endl
#define LOG_ERROR_0(msg)   std::cerr << ANSI_COLOR_RED << "[ERROR] " << ANSI_COLOR_RESET << msg << std::endl
#define LOG_DEBUG_0(msg)   std::cout << ANSI_COLOR_CYAN << "[DEBUG] " << ANSI_COLOR_RESET << msg << std::endl
#define LOG_SUCCESS_0(msg)                                                                                             \
    std::cout << ANSI_COLOR_GREEN << ANSI_BOLD << "[SUCCESS] " << msg << ANSI_COLOR_RESET < std::endl
#define LOG_FAIL_0(msg) std::cerr << ANSI_COLOR_RED << ANSI_BOLD << "[FAIL] " << ANSI_COLOR_RESET << msg << std::endl

// 使用boost::format的彩色输出宏（带参数版本）
#define LOG_INFO(fmt, ...)                                                                                             \
    std::cout << ANSI_COLOR_GREEN << "[INFO] " << ANSI_COLOR_RESET << (boost::format(fmt) % __VA_ARGS__).str()         \
              << std::endl

#define LOG_WARN(fmt, ...)                                                                                             \
    std::cout << ANSI_COLOR_YELLOW << "[WARN] " << ANSI_COLOR_RESET << (boost::format(fmt) % __VA_ARGS__).str()        \
              << std::endl

#define LOG_ERROR(fmt, ...)                                                                                            \
    std::cerr << ANSI_COLOR_RED << "[ERROR] " << ANSI_COLOR_RESET << (boost::format(fmt) % __VA_ARGS__).str()          \
              << std::endl

#define LOG_DEBUG(fmt, ...)                                                                                            \
    std::cout << ANSI_COLOR_CYAN << "[DEBUG] " << ANSI_COLOR_RESET << (boost::format(fmt) % __VA_ARGS__).str()         \
              << std::endl

#define LOG_SUCCESS(fmt, ...)                                                                                          \
    std::cout << ANSI_COLOR_GREEN << ANSI_BOLD << "[SUCCESS] " << ANSI_COLOR_RESET                                     \
              << (boost::format(fmt) % __VA_ARGS__).str() << std::endl

#define LOG_FAIL(fmt, ...)                                                                                             \
    std::cerr << ANSI_COLOR_RED << ANSI_BOLD << "[FAIL] " << ANSI_COLOR_RESET                                          \
              << (boost::format(fmt) % __VA_ARGS__).str() << std::endl

// 带背景色的重要信息宏
#define LOG_CRITICAL(fmt, ...)                                                                                         \
    std::cerr << ANSI_BG_RED << ANSI_COLOR_WHITE << ANSI_BOLD << "[CRITICAL] " << ANSI_COLOR_RESET                     \
              << (boost::format(fmt) % __VA_ARGS__).str() << std::endl

// 简化的彩色输出宏
#define PRINT_RED(fmt, ...)                                                                                            \
    std::cout << ANSI_COLOR_RED << (boost::format(fmt) % __VA_ARGS__).str() << ANSI_COLOR_RESET << std::endl
#define PRINT_GREEN(fmt, ...)                                                                                          \
    std::cout << ANSI_COLOR_GREEN << (boost::format(fmt) % __VA_ARGS__).str() << ANSI_COLOR_RESET << std::endl
#define PRINT_YELLOW(fmt, ...)                                                                                         \
    std::cout << ANSI_COLOR_YELLOW << (boost::format(fmt) % __VA_ARGS__).str() << ANSI_COLOR_RESET << std::endl
#define PRINT_BLUE(fmt, ...)                                                                                           \
    std::cout << ANSI_COLOR_BLUE << (boost::format(fmt) % __VA_ARGS__).str() << ANSI_COLOR_RESET << std::endl
#define PRINT_CYAN(fmt, ...)                                                                                           \
    std::cout << ANSI_COLOR_CYAN << (boost::format(fmt) % __VA_ARGS__).str() << ANSI_COLOR_RESET << std::endl
#define PRINT_MAGENTA(fmt, ...)                                                                                        \
    std::cout << ANSI_COLOR_MAGENTA << (boost::format(fmt) % __VA_ARGS__).str() << ANSI_COLOR_RESET << std::endl

// 无参数版本的彩色输出宏
#define PRINT_RED_0(msg)     std::cout << ANSI_COLOR_RED << msg << ANSI_COLOR_RESET << std::endl
#define PRINT_GREEN_0(msg)   std::cout << ANSI_COLOR_GREEN << msg << ANSI_COLOR_RESET << std::endl
#define PRINT_YELLOW_0(msg)  std::cout << ANSI_COLOR_YELLOW << msg << ANSI_COLOR_RESET << std::endl
#define PRINT_BLUE_0(msg)    std::cout << ANSI_COLOR_BLUE << msg << ANSI_COLOR_RESET << std::endl
#define PRINT_CYAN_0(msg)    std::cout << ANSI_COLOR_CYAN << msg << ANSI_COLOR_RESET << std::endl
#define PRINT_MAGENTA_0(msg) std::cout << ANSI_COLOR_MAGENTA << msg << ANSI_COLOR_RESET << std::endl

// 兼容性宏，用于已有代码
#define SUCCESS(msg)         LOG_SUCCESS_0(msg)
#define ERROR(msg)           LOG_ERROR_0(msg)
#define INFO(msg)            LOG_INFO_0(msg)
#define WARN(msg)            LOG_WARN_0(msg)
#define DEBUG(msg)           LOG_DEBUG_0(msg)

// 条件编译：在DEBUG模式下启用调试输出
#ifdef DEBUG
#define DBG_PRINT(fmt, ...) LOG_DEBUG(fmt, __VA_ARGS__)
#define DBG_PRINT_0(msg)    LOG_DEBUG_0(msg)
#else
#define DBG_PRINT(fmt, ...) ((void)0)
#define DBG_PRINT_0(msg)    ((void)0)
#endif

// 断言宏，带彩色输出
#define ASSERT_WITH_MSG(condition, fmt, ...)                                                                           \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            std::cerr << ANSI_BG_RED << ANSI_COLOR_WHITE << ANSI_BOLD << "[CRITICAL] " << ANSI_COLOR_RESET             \
                      << "Assertion failed: " << #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl;     \
            std::cerr << ANSI_BG_RED << ANSI_COLOR_WHITE << ANSI_BOLD << "[CRITICAL] " << ANSI_COLOR_RESET             \
                      << (boost::format(fmt) % __VA_ARGS__).str() << std::endl;                                        \
            std::abort();                                                                                              \
        }                                                                                                              \
    } while (0)

#define ASSERT_WITH_MSG_0(condition, msg)                                                                              \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            std::cerr << ANSI_BG_RED << ANSI_COLOR_WHITE << ANSI_BOLD << "[CRITICAL] " << ANSI_COLOR_RESET             \
                      << "Assertion failed: " << #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl;     \
            std::cerr << ANSI_BG_RED << ANSI_COLOR_WHITE << ANSI_BOLD << "[CRITICAL] " << ANSI_COLOR_RESET << msg      \
                      << std::endl;                                                                                    \
            std::abort();                                                                                              \
        }                                                                                                              \
    } while (0)
