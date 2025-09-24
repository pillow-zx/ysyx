#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

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

static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c) {
    if (user_handler) {
        Event ev = {0};  // 创建一个事件并将所有成员变量初始化为0
        switch (c->mcause) {
            case 0xb:
                ev.event = EVENT_YIELD;  // 事件类型为EVENT_YIELD
                ev.cause = c->mcause;
                ev.ref = c->mepc;  // 事件引用为mepc寄存器的值
                c->mepc += 4;      // 将mepc寄存器的值增加4，指向下一条指令
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
    // initialize exception entry
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

    // register event handler
    user_handler = handler;

    return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
    Context *c = (Context *)kstack.end - 1;  // 在栈顶分配一个Context结构体
    memset(c, 0, sizeof(Context));           // 将Context结构体的所有成员变量初始化为0

    c->gpr[2] = (uintptr_t)kstack.end;  // 设置栈指针寄存器（x2或sp）指向栈顶
    c->gpr[10] = (uintptr_t)arg;        // 设置第一个函数参数寄存器（x10或a0）为传入的参数arg
    c->mepc = (uintptr_t)entry;         // 设置程序计数器寄存器（mepc）为函数入口地址entry
    c->mstatus = 0x1800;                // 设置机器状态寄存器（mstatus），启用中断并设置特权级
    c->mcause = CAUSE_MACHINE_ECALL;    // 设置异常原因寄存器（mcause）为机器模式下的环境调用
    return c;                           // 返回初始化好的Context结构体指针
}

void yield() {
#ifdef __riscv_e
    asm volatile("li a5, -1; ecall");
#else
    asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() { return false; }

void iset(bool enable) {}
