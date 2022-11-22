
int strtod(char *c) {
    int num = 0;
    char digit;

    while((digit = *c) != 0) {
        num = (10 * num) + (digit - '0');
        c++;
    }
    
    return num;
}

int simple_strcmp(char *s0, char *s1) {
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
