#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

// 回调函数：获取一个待处理事件与相关上下文，返回处理后的上下文
static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c) {
    printf("Welcome to __am_irq_handle\n");
    if (user_handler) {
        Event ev = {0}; // 创建一个事件并将所有成员变量初始化为0
        switch (c->mcause) {
            default: ev.event = EVENT_ERROR; break;
        }

        for (int i = 0; i < NR_REGS; i++) {
            printf("%lx ", c->gpr[i]);
        }
        printf("\n%lx %lx %lx\n", c->mepc, c->mcause, c->mstatus);
        c = user_handler(ev, c);
        for (int i = 0; i < NR_REGS; i++) {
            printf("%lx ", c->gpr[i]);
        }
        printf("\n%lx %lx %lx\n", c->mepc, c->mcause, c->mstatus);
        assert(c != NULL);
    }

    return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *)) {
    // initialize exception entry 初始化异常入口为mtvec
    //  asm volatile：告诉编译器不要优化或删除这段内联汇编代码。
    //  "csrw mtvec, %0"：RISC-V汇编指令，将一个值写入mtvec寄存器。
    //  "r"(__am_asm_trap)：把__am_asm_trap的地址作为输入传递给汇编代码。
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
    
    user_handler = handler;

    return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
    return NULL;
}

void yield() {
#ifdef __riscv_e
    asm volatile("li a5, -1; ecall");
#else
    asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
    return false;
}

void iset(bool enable) {}
