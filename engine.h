#ifndef ENGINE_H
#define ENGINE_H

#include "config.h"

#define MAX_ARGS   (1 + 4)

typedef struct app_context app_context_t;
typedef struct command     command_t;
typedef struct sandbox     sandbox_t;

/* Defines individual commands */
typedef int(*command_executor_t)(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
typedef void (*command_printer_t)(command_t cmd);
typedef struct command {
    char *args[MAX_ARGS];     /* args[0] is the opcode */
    command_executor_t execute;
} command_t;

char *command_next(char **prog);
int command_decode(char *cmdstr, command_t *cmd);
int command_match(command_t cmd, int idx);

/* Defines types of valid commands */
typedef struct command_type {
    char *opcode;
    int n_args;
    const command_printer_t  print;
    const command_executor_t execute;
    long *executor_sizep;
} command_type_t;

extern command_type_t vocabulary[];
extern int            vocabulary_size;

/* Define sandboxes for running programs */
typedef struct variable {
    char name[MAX_VAR_SZ];
    long value;
} var_t;

typedef struct array {
    char name[MAX_VAR_SZ];
    void *base;
} array_t;

typedef char arena_t[ARENA_SIZE];
extern long     n_arenas_used;
typedef void *(*trampoline_fn_t)(const sandbox_t *);
typedef int (*app_executor_t)(const sandbox_t *);

typedef struct app_context {
    var_t vars[MAX_VARS];
    long n_vars;

    array_t arrays[MAX_ARRS];
    long n_arrays;

    arena_t darena;
    long d_used_bytes;

    /* Arguments used in callbacks */
    long n_cmds;    /* Number of commands to execute */
    long alloc_size;        /* Size of longs for allocation request */
    const char *print_var;
    long print_val;
    long cur_code_idx;

} app_context_t;

typedef struct sandbox {
    char cmd_args_arena[MAX_CMDS * MAX_CMD_SZ];
    long cmd_args_used_bytes;
    command_t cmds[MAX_CMDS];
    long n_cmds;

    void *carena;        /* Generated code is put in the arena */
    long c_used_bytes;

    void *trampoline_carena;
    long t_carena_used_bytes;

    app_executor_t execute;
    app_context_t *ctx;

    /* List of callbacks into the Engine */
    trampoline_fn_t allocator;
    trampoline_fn_t print_var;

#if CONFIG_COMP
    unsigned comp_id;
    /* Entry trampoline */
    void *(*trampoline_call)(void *contexts, int comp_id,
                                trampoline_fn_t fn,
                                void *args);
    void (*trampoline_return)(void *contexts, void *ret);
    void *contexts;
#endif
} sandbox_t;

int engine_init();

sandbox_t *sandbox_alloc_init();
int sandbox_init(sandbox_t *box);
int sandbox_add_command(sandbox_t *box, command_t cmd);
int sandbox_execute(sandbox_t *box, long n_cmds);

#endif /* ENGINE_H */