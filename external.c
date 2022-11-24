#include "external.h"

#ifdef SEL4
#include <sel4/sel4.h>
#include <sel4platsupport/platsupport.h>
#include <stdio.h>
#include <sys/mman.h>
#include "mmap_override.h"

void protect_region(void *addr, int size, int read, int write, int exec) {

}

void prints(char *str, int size) {
  printf("%s", str);
}

void *mmap_region(void *start, long len, int read, int write, int exec) {
  int prot = 0;
  if(read)
    prot |= PROT_READ;
  if(write)
    prot |= PROT_WRITE;
  if(exec)
    prot |= PROT_EXEC;
  
  return mmap_override(start, len, prot, MAP_ANONYMOUS, -1, 0);
}

void __attribute__((noreturn)) program_exit(int code) {
  seL4_TCB_Suspend(seL4_CapInitThreadTCB);
}

int platform_specific_setup() {
  /* Setup serial output via seL4_Debug_PutChar */
  if (platsupport_serial_setup_bootinfo_failsafe()) {
    /* Error occured during setup => terminate */
    return 1;
  }
  return 0;
}

#else /* Assuming Linux */
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

void *mmap_region(void *start, long len, int read, int write, int exec) {
  int prot = 0;
  if(read)
    prot |= PROT_READ;
  if(write)
    prot |= PROT_WRITE;
  if(exec)
    prot |= PROT_EXEC;

  return mmap(start, len, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void __attribute__((noreturn)) program_exit(int code) {
  _exit(code);
}

int platform_specific_setup() {

}
#endif