#include <device.h>
#include <signal.h>
#include <sys/time.h>
#include <cstring>


#define MAX_HANDLER 8

static alarm_handler_t handler[MAX_HANDLER] = {};
static int index_ = 0;

void add_alarm_handle(alarm_handler_t func) {
    assert(index_ < MAX_HANDLER);
    handler[index_++] = func;
}

static void alarm_sig_handler(int signum) {
    for (int i = 0; i < index_; i++) {
        if (handler[i]) {
            handler[i]();
        }
    }
}

void init_alarm() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alarm_sig_handler;
    int ret = sigaction(SIGALRM, &sa, nullptr);
    assert(ret == 0);

    struct itimerval timer = {};
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000 / TIMER_HZ;  // 1s
    timer.it_interval = timer.it_value; // Repeat every 100 ms
    ret = setitimer(ITIMER_REAL, &timer, nullptr);
    assert(ret == 0);
}