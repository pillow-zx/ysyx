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
void init_log(const char* log_file);
void init_mem();
void init_difftest(char* ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm();

static void welcome() {
    Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
    IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
    Log("Build time: %s, %s", __TIME__, __DATE__);
    printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
    printf("For help, type \"help\"\n");
    // Log("Exercise: Please remove me in the source code and compile NEMU again.");
    // assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char* log_file = NULL;
static char* diff_so_file = NULL;
static char* img_file = NULL;
static int difftest_port = 1234;

static long load_img() {
    if (img_file == NULL) {
        Log("No image is given. Use the default build-in image.");
        return 4096; // built-in image size
    }

    FILE* fp = fopen(img_file, "rb");
    Assert(fp, "Can not open '%s'", img_file);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    Log("The image is %s, size = %ld", img_file, size);

    fseek(fp, 0, SEEK_SET);
    int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
    assert(ret == 1);

    fclose(fp);
    return size;
}

static int parse_args(int argc, char* argv[]) {     // 使用static修饰函数，表示该函数只能在本文件中使用
    const struct option table[] = {                 // 使用const修饰结构体，表示该结构体的值不能被修改
      {"batch"    , no_argument      , NULL, 'b'},
      {"log"      , required_argument, NULL, 'l'},
      {"diff"     , required_argument, NULL, 'd'},
      {"port"     , required_argument, NULL, 'p'},
      {"help"     , no_argument      , NULL, 'h'},
      {0          , 0                , NULL,  0 },
    };
    int o;
    /* 对getopt_long函数的详细说明见： file:///home/waysorry/user/NemuNote/function/getopt_long.md */
    while ((o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {             // 逐个获取命令行参数并返回对应的ascii码
        switch (o) {                                                                    // 根据对应的ascii码进行switch判断
        case 'b': sdb_set_batch_mode(); break;
        case 'p': sscanf(optarg, "%d", &difftest_port); break;
        case 'l': log_file = optarg; break;
        case 'd': diff_so_file = optarg; break;
        case 1: img_file = optarg; return 0;
        default:                                                                        // 处理错误参数
            printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
            printf("\t-b,--batch              run with batch mode\n");                  //  以批处理模式运行
            printf("\t-l,--log=FILE           output log to FILE\n");                   //  输出日志到文件
            printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");   //  运行DiffTest，使用参考实现REF_SO
            printf("\t-p,--port=PORT          run DiffTest with port PORT\n");          //  运行DiffTest，使用端口PORT
            printf("\n");
            exit(0);
        }
    }
    return 0;
}

void init_monitor(int argc, char* argv[]) {
    /* Perform some global initialization. */

    /* Parse arguments. 解析命令行参数 */
    parse_args(argc, argv);  //parse ：解析

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

    /* Load the image to memory. This will overwrite the built-in image. 将图像加载到内存中。这将覆盖内置图像 */
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
