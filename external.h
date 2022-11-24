
void protect_region(void *addr, long size, int read, int write, int exec);
void prints(char *str, long size);
void *mmap_region(void *start, long len, int read, int write, int exec);
void __attribute__((noreturn)) program_exit(int code);
int platform_specific_setup();
