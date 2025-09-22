#include <am.h>
#include <klib.h>
#include <rtthread.h>
#include <stdint.h>
#include "rtdef.h"


// wrapper的参数
struct rt_thread_wrapper_args {
    void *tentry;
    void *parameter;
    void *texit;
};

// kcontext实际执行的函数
static void rt_thread_wrapper(void *args) {
    struct rt_thread_wrapper_args *wrapper_args = (struct rt_thread_wrapper_args *)args;


    void (*entry_func)(void *) = (void (*)(void *))wrapper_args->tentry;
    entry_func(wrapper_args->parameter);


    void (*exit_func)(void) = (void (*)(void))wrapper_args->texit;
    exit_func();
}

// 由于cte机制的存在，rt_hw_context_switch等函数并不是直接调用的ev_handler函数
// 不能直接把rt_hw_context_switch等函数的参数传递给ev_handler，所以此处使用全局变量
static rt_ubase_t *g_from_context = NULL;
static rt_ubase_t *g_to_context = NULL;

// 根据am代码，ev_handler返回的上下文就是即将恢复的上下文,且目前只需实现对yield事件的识别
// 所以当识别到yield事件时需要返回g_to_context指向的上下文
static Context *ev_handler(Event e, Context *c) {
    switch (e.event) {
        case EVENT_YIELD:
            if (g_from_context != NULL) {
                *g_from_context = (rt_ubase_t)c;
            }
            if (g_to_context != NULL) {
                return (Context *)*g_to_context;
            }
            return c;
        default: printf("Unhandled event ID = %d\n", e.event); assert(0);
    }
    return c;
}

void __am_cte_init() { cte_init(ev_handler); }

void rt_hw_context_switch_to(rt_ubase_t to) {
    g_from_context = NULL;
    g_to_context = (rt_ubase_t *)to;
    yield();
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
    g_from_context = (rt_ubase_t *)from;
    g_to_context = (rt_ubase_t *)to;
    yield();
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) { assert(0); }

/* 创建线程上下文 */
// 因为一个线程有两个函数tentry(parameter)和texit需要运行，所以在创建上下文时需要保存两个函数指针和对应参数
// 因为在am中用于创建上下文的kcontext函数实际上就是保存了要运行的一个函数的地址，不可能直接传出两个函数
// 为了解决这一点创建了一个新函数wrapper，把tentry(parameter)和texit函数放到wrapper函数内，将wrapper函数放入上下文中，
// 当线程运行时直接使用wrapper函数实际上就是先后调用了tentry(parameter)和texit函数
// 又因为wrapper函数要想使用tentry和textit函数就需要把这两个函数作为传参传入wrapper中，但是kcontext函数只能传递一个参数，但是实际上需要传递tentry,parameter，texit三个参数
// 为了避免使用全局变量时竟态情况的放生，使用一个栈来保存这些参数，并传入kcontext中
rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
    uintptr_t aligned_stack = (uintptr_t)stack_addr & ~(sizeof(uintptr_t) - 1);

    aligned_stack -= sizeof(struct rt_thread_wrapper_args);
    struct rt_thread_wrapper_args *args = (struct rt_thread_wrapper_args *)aligned_stack;

    args->tentry = tentry;
    args->parameter = parameter;
    args->texit = texit;

    Area kstack = {.start = (void *)0, .end = (void *)aligned_stack};

    Context *ctx = kcontext(kstack, rt_thread_wrapper, (void *)args);

    return (rt_uint8_t *)ctx;
}
