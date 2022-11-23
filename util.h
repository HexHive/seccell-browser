#ifndef UTIL_H
#define UTIL_H

#ifndef NULL
#define NULL ((void *)0)
#endif

// TODO: Handle hexadecimal numbers
static inline  __attribute__((always_inline))
int util_strtod(const char *c) {
    int num = 0;
    char digit;

    while((digit = *c) != 0) {
        num = (10 * num) + (digit - '0');
        c++;
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

#endif /* UTIL_H */