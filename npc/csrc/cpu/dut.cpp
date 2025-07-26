#include <cpu.h>
#include <dlfcn.h>

void (*ref_difftest_memcpy)(uint32_t addr, void *buf, unsigned long n, bool direction) = nullptr;
void (*ref_difftest_regcpy)(void *dut, bool direction) = nullptr;
void (*ref_difftest_exec)(uint64_t n) = nullptr;


static bool is_skip_ref = false;
static int skip_dut_nr_inst = 0;

void difftest_skip_ref() {
    is_skip_ref = true;
    skip_dut_nr_inst = 0;
}

void difftest_skip_dut(int nr_ref, int nr_dut) {
    skip_dut_nr_inst += nr_dut;

    while (nr_ref-- > 0) {
        ref_difftest_exec(1);
    }
}

void init_difftest(char *ref_so_file, long img_size, int port) {
    assert(ref_so_file != nullptr);

    void *handle = dlopen(ref_so_file, RTLD_LAZY);
    assert(handle != nullptr);

    ref_difftest_memcpy = (void (*)(uint32_t, void *, unsigned long, bool))dlsym(handle, "difftest_memcpy"); // 获取动态链接库中的函数地址
    assert(ref_difftest_memcpy);

    ref_difftest_exec = (void (*)(uint64_t))dlsym(handle, "difftest_exec");
    assert(ref_difftest_exec);

    void (*ref_difftest_init)(int) = (void (*)(int))dlsym(handle, "difftest_init");
    assert(ref_difftest_init);


    ref_difftest_init(port);

    ref_difftest_memcpy(0x80000000, (void *)0x80000000, img_size, false); // 将程序镜像从 host 复制到 ref 模型中

    ref_difftest_regcpy((void *)0x80000000, false); // 将寄存器从 host 复制到 ref 模型中
}
