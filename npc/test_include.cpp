#include "tools.h"  // 测试：不使用相对路径，tools.h 依赖较少

int main() {
    // 这个文件应该能找到 tools.h，因为 -I 参数指定了搜索路径
    return 0;
}
