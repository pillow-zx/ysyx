#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>
#include <stdio.h>

#define CAUSE_MISALIGNED_FETCH    0
#define CAUSE_FETCH_ACCESS        1
#define CAUSE_ILLEGAL_INSTRUCTION 2
#define CAUSE_BREAKPOINT          3
#define CAUSE_MISALIGNED_LOAD     4
#define CAUSE_LOAD_ACCESS         5
#define CAUSE_MISALIGNED_STORE    6
#define CAUSE_STORE_ACCESS        7
#define CAUSE_USER_ECALL          8
#define CAUSE_SUPERVISOR_ECALL    9
#define CAUSE_MACHINE_ECALL       11
#define CAUSE_FETCH_PAGE_FAULT    12
#define CAUSE_LOAD_PAGE_FAULT     13
#define CAUSE_STORE_PAGE_FAULT    15

// 回调函数：获取一个待处理事件与相关上下文，返回处理后的上下文
static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c) {
    if (user_handler) {
        Event ev = {0};  // 创建一个事件并将所有成员变量初始化为0
        switch (c->mcause) {
            case 0xb:
                ev.event = EVENT_YIELD;  // 事件类型为EVENT_YIELD
                ev.cause = c->mcause;
                ev.ref = c->mepc;  // 事件引用为mepc寄存器的值
                break;
            default: ev.event = EVENT_ERROR; break;
        }
        c = user_handler(ev, c);
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

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) { return NULL; }

void yield() {
#ifdef __riscv_e
    asm volatile("li a5, -1; ecall");
#else
    asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() { return false; }

void iset(bool enable) {}
