#ifndef UTIL_H
#define UTIL_H

#ifndef NULL
#define NULL ((void *)0)
#endif
typedef unsigned long uintptr_t;

static inline __attribute__((always_inline))
char to_char(int n) {
    if(n < 10)
        return '0' + n;
    else
        return 'a' + n - 10;
}

static inline __attribute__((always_inline))
int from_char(char c) {
    if('0' <= c && c <= '9')
        return c - '0';
    else if('A' <= c && c <= 'Z')
        return c - 'A' + 10;
    else if('a' <= c && c <= 'z')
        return c - 'a' + 10;
    
    return 0;
}

static inline  __attribute__((always_inline))
int util_strtod(const char *s) {
    int num = 0, base = 10;
    char digit;

    if(s[0] == '0' && s[1] == 'x') {
        base = 16;
        s += 2;
    }

    while((digit = *s) != 0) {
        num = (base * num) + from_char(digit);
        s++;
    }
    
    return num;
}

static inline  __attribute__((always_inline))
int util_strcmp(const char *s0, const char *s1)  {
    char c0, c1;

    do {
        c0 = *s0;
        c1 = *s1;

        if(c0 != c1)
            return c0 - c1;

        s0++;
        s1++;
    } while(!!c0  && !!c1);

    return 0;
}

static inline __attribute__((always_inline))
void util_strcpy(char *dst, const const char *src)  {
    char c;
    do {
        c = *src;
        *dst = c;

        src++;
        dst++;
    } while(c);
}

static inline __attribute__((always_inline))
void util_memcpy(char *dst, char *src, int size)  {
    for(int i = 0; i < size; i++) 
        *(dst++) = *(src++);
}

static inline __attribute__((always_inline))
int util_snprintf(char *buf, int max, const char *format, uintptr_t args[]) {
    char c;
    int state = 0, args_idx = 0, out_idx = 0;

    while(((c = *format) != '\0') && (out_idx < max - 1)) {
        if(state == 0) { 
            /* Non-format specifier */
            if(c == '%')
                state = 1;
            else if(c == '\\')
                state = 2;
            else
                *(buf + out_idx++) = c;
        } else if (state == 1) {
            if(c == 's') { /* Output string */
                char *src = (char *)args[args_idx++];
                do {
                    c = *src;
                    *(buf + out_idx++) = c;

                    src++;
                } while(c);
                out_idx -= 1;
            } else if(c == 'd' || c == 'x') {
                int base = (c == 'd')? 10 : 16;
                int val = (int)args[args_idx++];
                char ptit_buf[12];
                int ptit_buf_idx = 0, neg = 0;
                if((val < 0) && (base != 16)) {
                    neg = 1;
                    val = -val;
                }
                do {
                    int digit = (base == 16)? val & 0xf: val % base;
                    ptit_buf[ptit_buf_idx++] = to_char(digit);
                    val = (base == 16)? (unsigned)val >> 4: val / base;
                } while(val != 0);
                if(neg)
                    ptit_buf[ptit_buf_idx++] = '-';
                do {
                    *(buf + out_idx++) = ptit_buf[--ptit_buf_idx];
                } while(ptit_buf_idx > 0);
            } else
                return -1;
            
            state = 0;
        } else if(state == 2) {
            if(c == 'n')
                *(buf + out_idx++) = '\n';
            else if (c == 't')
                *(buf + out_idx++) = '\t';
            else if (c == '0')
                *(buf + out_idx++) = '\0';
            else
                return -1;

            state = 0;
        } else
            return -1;

        format++;
    }

    *(buf + out_idx++) = '\0';

    return out_idx;
}

#endif /* UTIL_H */