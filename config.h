#ifndef CONFIG_H
#define CONFIG_H

#define MAX_CMDS   0x100
#define MAX_SANDBOXES 0x10
#define ARENA_SIZE 0x2000
#define MAX_VARS   0x10
#define MAX_VAR_SZ 0x10
#define MAX_ARRS   0x10

#ifdef SEL4
#include "seL4-playground/gen_config.h"

#if CONFIG_EVAL_TYPE_UNCOMP
#define COMP 0
#elif CONFIG_EVAL_TYPE_COMP
#define COMP 1
#endif

#else  /* Assuming Linux */
#define COMP 0
#endif

#define TRY_LEAK_MAIN_SECRET
#define TRY_LEAK_OTHER_PROG_SECRET

#define DEBUG

#endif /* CONFIG_H */