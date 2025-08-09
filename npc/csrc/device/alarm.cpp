#include <sys/time.h>
#include <alarm.h> 
#include <signal.h>
#include <device.h>
#include <map.h>


#define MAX_HANDLER 8

static alarm_handler_t handler[MAX_HANDLER] = {};
static int idx = 0;


void add_alarm_handle(alarm_handler_t h) {
    assert(idx < MAX_HANDLER);
    handler[idx++] = h;
}

static void alarm_sig_handler(int signum) {
    for (int i= 0; i < idx; i++) {
        if (handler[i]) {
            handler[i]();
        }
    }
}

void init_alarm() {
    struct sigaction s;
    memset(&s, 0, sizeof(s));
    s.sa_handler = alarm_sig_handler;
    int ret = sigaction(SIGALRM, &s, nullptr);
    assert(ret == 0);

    struct itimerval it = {};
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 1000000 / TIMER_HZ; // 每秒
    it.it_interval = it.it_value; // 重复间隔
    ret = setitimer(ITIMER_REAL, &it, nullptr);
    assert(ret == 0);
}