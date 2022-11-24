
void protect_region(void *addr, int size, int read, int write, int exec);
void prints(char *str, int size);
void *mmap_region(void *start, size_t len, int read, int write, int exec);
