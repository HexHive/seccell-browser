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
#else /* Assuming seL4 */

#endif