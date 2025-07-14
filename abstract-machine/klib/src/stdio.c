#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
    panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *p = out;
    const char *s = fmt;

    while (*p) {
        if (*p == '%' && (*(p + 1) == 'd' || *(p + 1) == 's')) {
            p++;
            if (*p == 'd') {
                int value = va_arg(args, int);
                out += snprintf(out, 20, "%d", value);
            } else if (*p == 's') {
                char *sval = va_arg(args, char *);
                while (*sval) {
                    *out++ = *sval++;
                }
            }
            p++;
        } else {
            *out++ = *p++;
        }
    }
    *out = '\0';
    va_end(args);
    return out - p;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
