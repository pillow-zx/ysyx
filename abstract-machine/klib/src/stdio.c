#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
    panic("Not implemented");
}

static int getlength(int num) {
    int len = 0;
    if (num == 0)
        return 1;
    while (num != 0) {
        num /= 10;
        len++;
    }
    return len;
}

static void reverse_string(char *str, int len, int is_negative) {
    int start = is_negative ? 1 : 0;
    int end = len - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static char *int_to_string(int num) {
    int len = getlength(num);
    int i = num < 0 ? 1 : 0; // start from index 1 if negative
    static char str[20];
    if (num < 0) {
        len++;      // for negative sign
        num = -num; // make it positive for conversion
        str[0] = '-';
    }
    str[len] = '\0';
    
    // Special case for 0
    if (num == 0) {
        str[i] = '0';
        return str;
    }
    
    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }
    reverse_string(str, len, str[0] == '-');
    return str;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    char *p = out;
    char *temp = (char *)fmt;
    while (*temp) {
        if (*temp == '%') {
            temp++;
            switch (*temp) {
                case 'd': {
                    int num = va_arg(ap, int);
                    char *str = int_to_string(num);
                    while (*str) {
                        *p++ = *str++;
                    }
                    break;
                }
                case 's': {
                    char *str = va_arg(ap, char *);
                    while (*str) {
                        *p++ = *str++;
                    }
                    break;
                }
                default:
                    // Handle unknown format specifier by just copying it
                    *p++ = '%';
                    *p++ = *temp;
                    break;
            }
        } else {
            // Copy regular characters
            *p++ = *temp;
        }
        temp++;
    }
    *p = '\0';  // Only set null terminator at the end
    return (p - out);
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vsprintf(out, fmt, ap);
    va_end(ap);
    return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
