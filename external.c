#include "external.h"

#if __linux__
#include <sys/mman.h>
#include <unistd.h>

void protect_region(void *addr, int size, int read, int write, int exec) {
  int prot = 0;
  if(read)
    prot |= PROT_READ;
  if(write)
    prot |= PROT_WRITE;
  if(exec)
    prot |= PROT_EXEC;

  mprotect(addr, size, prot);
}

void prints(char *str, int size) {
  write(STDOUT_FILENO, str, size);
}

void *mmap_region(void *start, size_t len, int read, int write, int exec) {
  int prot = 0;
  if(read)
    prot |= PROT_READ;
  if(write)
    prot |= PROT_WRITE;
  if(exec)
    prot |= PROT_EXEC;

  return mmap(start, len, prot, MAP_PRIVATE, -1, 0);
}

#else /* Assuming seL4 */
#include "mmap_override.h"

void protect_region(void *addr, int size, int read, int write, int exec) {

}

void prints(char *str, int size) {
  printf("%s", str);
}

void *mmap_region(void *start, size_t len, int read, int write, int exec) {
  int prot = 0;
  if(read)
    prot |= PROT_READ;
  if(write)
    prot |= PROT_WRITE;
  if(exec)
    prot |= PROT_EXEC;
  
  return mmap_override(start, len, prot, flags, fd, off);
}
#endif