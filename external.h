#include "config.h"
void protect_region(void *addr, long size, int read, int write, int exec);
void prints(char *str, long size);
void *mmap_region(void *start, long len, int read, int write, int exec);
void __attribute__((noreturn)) program_exit(int code);
int platform_specific_setup(void **contexts_p);
int allocate_compartment();
int compartment_permit(int comp_id, void *addr, int read, int write, int exec);
static int switch_to_compartment(int comp_id);

#ifdef SEL4
#include "seccells.h"
#include <scthreads.h>
static inline __attribute__((always_inline))
int switch_to_compartment(int comp_id) {
  jals(comp_id, entry);
entry:
  entry();
}
#endif