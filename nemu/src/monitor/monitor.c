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

#include <isa.h>
#include <memory/paddr.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm();

static void welcome() {
    Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
    IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
                            "to record the trace. This may lead to a large log file. "
                            "If it is not necessary, you can disable it in menuconfig"));
    Log("Build time: %s, %s", __TIME__, __DATE__); // __TIME__和__DATE__是预定义宏，分别表示编译时间和编译日期
    printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
    printf("For help, type \"help\"\n");
    // Log("Exercise: Please remove me in the source code and compile NEMU again.");
    // assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = "/home/waysorry/ysyx/ysyx-workbench/am-kernels/tests/cpu-tests/build/add-riscv32-nemu.bin";
static int difftest_port = 1234;

static long load_img() {
    // 检查 img_file 是否为 NULL
    if (img_file == NULL) {
        Log("No image is given. Use the default build-in image.");
        return 4096; // built-in image size
    }

    // 以二进制方式打开镜像文件
    FILE *fp = fopen(img_file, "rb");
    Assert(fp, "Can not open '%s'", img_file);

    // 获取镜像文件的大小
    // ftell(fp) 返回当前文件指针的位置，fseek(fp, 0, SEEK_END) 将文件指针移动到文件末尾
    // 通过 ftell(fp) 获取文件大小
    // SEEK_END 表示从文件末尾开始计算偏移
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    Log("The image is %s, size = %ld", img_file, size);

    // 检查镜像大小是否超过物理内存大小
    fseek(fp, 0, SEEK_SET);
    int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp); // 将镜像文件内容读入到物理内存中
    assert(ret == 1);

    // 关闭文件
    fclose(fp);
    return size;
}

#ifdef CONFIG_FTRACE
#include <elf.h>
char *ftrace_file = NULL;                // 用于存储ELF格式的镜像文件路径
Elf32_Ehdr *ftrace_file_header;          // ELF文件头结构体
Elf32_Shdr *ftrace_file_sections = NULL; // ELF节头结构体数组
char *ftrace_file_strtab = NULL;         // ELF符号名字符串表
Elf32_Sym *ftrace_file_symtab = NULL;    //  ELF符号表
int ftrace_file_symtab_num = 0;          // 符号表条目数量
// 解析elf文件
static void ftrace_elf_init(char *ftrace_file) {
    if (ftrace_file == NULL) {
        Log("No ftrace file is given.");
        return;
    }

    FILE *fp = fopen(ftrace_file, "rb");
    Assert(fp, "Can not open '%s'", ftrace_file);

    // 读取ELF文件头
    ftrace_file_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    Assert(ftrace_file_header, "Failed to allocate memory for ELF header");
    size_t ret = fread(ftrace_file_header, sizeof(Elf32_Ehdr), 1, fp);
    Assert(ret == 1, "Failed to read ELF header from '%s'", ftrace_file);

    // 检查ELF文件头的魔数是否正确
    if (ftrace_file_header->e_ident[EI_MAG0] != ELFMAG0 || ftrace_file_header->e_ident[EI_MAG1] != ELFMAG1 ||
        ftrace_file_header->e_ident[EI_MAG2] != ELFMAG2 || ftrace_file_header->e_ident[EI_MAG3] != ELFMAG3) {
        fclose(fp);
        return;
    }

    // 导航到节头表
    if (ftrace_file_header->e_shoff == 0 || ftrace_file_header->e_shentsize == 0) {
        fclose(fp);
        return;
    }
    fseek(fp, ftrace_file_header->e_shoff, SEEK_SET);

    // 读取节头表
    ftrace_file_sections = malloc(ftrace_file_header->e_shnum * sizeof(Elf32_Shdr));
    Assert(ftrace_file_sections, "Failed to allocate memory for section headers");
    ret = fread(ftrace_file_sections, sizeof(Elf32_Shdr), ftrace_file_header->e_shnum, fp);
    Assert(ret == ftrace_file_header->e_shnum, "Failed to read section headers from '%s'", ftrace_file);

    // 读取节头符号表和对应的字符串表
    for (int i = 0; i < ftrace_file_header->e_shnum; i++) {
        if (ftrace_file_sections[i].sh_type == SHT_SYMTAB) {
            // 读取符号表
            fseek(fp, ftrace_file_sections[i].sh_offset, SEEK_SET);
            ftrace_file_symtab = (Elf32_Sym *)malloc(ftrace_file_sections[i].sh_size);
            Assert(ftrace_file_symtab, "Failed to allocate memory for symbol table");
            ret = fread(ftrace_file_symtab, ftrace_file_sections[i].sh_size, 1, fp);
            Assert(ret == 1, "Failed to read symbol table from '%s'", ftrace_file);
            // 计算符号表条目数量
            ftrace_file_symtab_num = ftrace_file_sections[i].sh_size / sizeof(Elf32_Sym);

            // 读取对应的符号名字符串表
            int strtab_index = ftrace_file_sections[i].sh_link;
            if (strtab_index < ftrace_file_header->e_shnum) {
                fseek(fp, ftrace_file_sections[strtab_index].sh_offset, SEEK_SET);
                ftrace_file_strtab = (char *)malloc(ftrace_file_sections[strtab_index].sh_size);
                Assert(ftrace_file_strtab, "Failed to allocate memory for symbol string table");
                ret = fread(ftrace_file_strtab, ftrace_file_sections[strtab_index].sh_size, 1, fp);
                Assert(ret == 1, "Failed to read symbol string table from '%s'", ftrace_file);
            }
            break; // 找到符号表后退出循环
        }
    }

    // 关闭文件
    Log("%s: ftrace file loaded successfully", ftrace_file);
    fclose(fp);
}
#endif // CONFIG_FTRACE

static int parse_args(int argc, char *argv[]) { // 使用static修饰函数，表示该函数只能在本文件中使用
    const struct option table[] = {
        // 使用const修饰结构体，表示该结构体的值不能被修改
        {"batch", no_argument, NULL, 'b'},       // 批处理模式
        {"log", required_argument, NULL, 'l'},   // 日志文件
        {"diff", required_argument, NULL, 'd'},  // diff文件
        {"port", required_argument, NULL, 'p'},  // 端口
        {"help", no_argument, NULL, 'h'},        // 帮助
        {"image", required_argument, NULL, 'i'}, // 镜像文件
        {0, 0, NULL, 0},                         // 结束标志
    };
    int o;
    /* 对getopt_long函数的详细说明见： file:///home/waysorry/user/NemuNote/function/getopt_long.md */
    while ((o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) { // 逐个获取命令行参数并返回对应的ascii码
        switch (o) {                                                        // 根据对应的ascii码进行switch判断
            case 'b': sdb_set_batch_mode(); break;
            case 'p': sscanf(optarg, "%d", &difftest_port); break;
            case 'l': log_file = optarg; break;
            case 'd': diff_so_file = optarg; break;
            case 'i':
                IFDEF(CONFIG_FTRACE, ftrace_file = optarg, ftrace_elf_init(ftrace_file));
                break;
            case 1: img_file = optarg; return 0;
            default: // 处理错误参数
                printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
                printf("\t-b,--batch              run with batch mode\n"); //  以批处理模式运行
                printf("\t-l,--log=FILE           output log to FILE\n");  //  输出日志到文件
                printf(
                    "\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n"); //  运行DiffTest，使用参考实现REF_SO
                printf("\t-p,--port=PORT          run DiffTest with port PORT\n");     //  运行DiffTest，使用端口PORT
                printf("\t-i,--image=FILE         load image from FILE\n");            //  从文件加载镜像
                printf("\t-h,--help              display this help and exit\n");       //  显示帮助信息并退出
                printf("\n");
                exit(0);
        }
    }
    return 0;
}

void init_monitor(int argc, char *argv[]) {
    /* Perform some global initialization. */

    /* Parse arguments. 解析命令行参数 */
    parse_args(argc, argv); //parse ：解析

    /* Set random seed. 设置随机数种子 */
    init_rand();

    /* Open the log file. 初始化日志文件 */
    init_log(log_file);

    /* Initialize memory. 初始化内存 */
    init_mem();

    /* Initialize devices. 初始化设备 */
    IFDEF(CONFIG_DEVICE, init_device());

    /* Perform ISA dependent initialization. 执行 ISA（即指令集架构） 依赖初始化 */
    init_isa();

    /* Load the image to memory. This will overwrite the built-in image. 将一个有意义的客户程序从镜像文件读入到内存, 覆盖刚才的内置客户程序, 并返回镜像文件大小 */
    long img_size = load_img();

    /* Initialize differential testing. 初始化不同测试 */
    init_difftest(diff_so_file, img_size, difftest_port);

    /* Initialize the simple debugger. 初始化简单debug */
    init_sdb();

    IFDEF(CONFIG_ITRACE, init_disasm());

    /* Display welcome message. 显示欢迎信息 */
    welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
    extern char bin_start, bin_end;
    size_t size = &bin_end - &bin_start;
    Log("img size = %ld", size);
    memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
    return size;
}

void am_init_monitor() {
    init_rand();
    init_mem();
    init_isa();
    load_img();
    IFDEF(CONFIG_DEVICE, init_device());
    welcome();
}
#endif
