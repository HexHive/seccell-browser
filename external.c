#include "external.h"

#ifdef SEL4
#include <sel4/sel4.h>
#include <sel4platsupport/platsupport.h>
#include <stdio.h>
#include <sys/mman.h>
#include <seccells.h>
#include <scthreads.h>
#include "mmap_override.h"


void protect_region(void *addr, long size, int read, int write, int exec) {
  int prot = 0;
  if(read)
    prot |= RT_R;
  if(write)
    prot |= RT_W;
  if(exec)
    prot |= RT_X;

  prot(addr, prot);
}

void prints(char *str, long size) {
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

#define CONTEXT_VADDR 0xF000000
int platform_specific_setup(void **contexts_p) {
  /* Setup serial output via seL4_Debug_PutChar */
  if (platsupport_serial_setup_bootinfo_failsafe()) {
    /* Error occured during setup => terminate */
    return 1;
  }

  seL4_BootInfo *info = platsupport_get_bootinfo();
  *contexts_p = scthreads_init_contexts(info, (void *)CONTEXT_VADDR, MAX_SANDBOXES + 2);

  return 0;
}

int allocate_compartment() {
  seL4_RISCV_RangeTable_AddSecDiv_t ret;

  ret = seL4_RISCV_RangeTable_AddSecDiv(seL4_CapInitThreadVSpace);
  if (unlikely(ret.error != seL4_NoError)) {
    return 1;
  }
  return ret.id;
}

int compartment_permit(int comp_id, void *addr, int read, int write, int exec) {
  int perm = 0;
  if(read)
    perm |= RT_R;
  if(write)
    perm |= RT_W;
  if(exec)
    perm |= RT_X;
  seL4_Error err;
  
  err = seL4_RISCV_RangeTable_GrantSecDivPermissions(seL4_CapInitThreadVSpace,
                                                     (seL4_Word)comp_id,
                                                     (seL4_Word)addr, 
                                                     perm);
  if (unlikely(err != seL4_NoError)) {
    return 1;
  }
  return 0;
}

#else /* Assuming Linux */
#include <sys/mman.h>
#include <unistd.h>

void protect_region(void *addr, long size, int read, int write, int exec) {
  int prot = 0;
  if(read)
    prot |= PROT_READ;
  if(write)
    prot |= PROT_WRITE;
  if(exec)
    prot |= PROT_EXEC;

  mprotect(addr, size, prot);
}

void prints(char *str, long size) {
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

int platform_specific_setup(void **contexts_p) {
  return 0;
}

int allocate_compartment() {
  return 0; 
}

int compartment_permit(int comp_id, void *addr, int read, int write, int exec) {
  return 0;
}

void *compartment_call(int comp_id,void *(*start_routine)(void *),void *restrict args) {
  return start_routine(args);
}
int switch_to_compartment(int comp_id) {
  return 0;
}
#endif