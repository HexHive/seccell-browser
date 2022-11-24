#ifndef ENGINE_H
#define ENGINE_H

#define MAX_ARGS   (1 + 4)
#define MAX_CMDS   0x100
#define MAX_ARENAS 0x10
#define ARENA_SIZE 0x4000
#define MAX_VARS   0x10
#define MAX_VAR_SZ 0x10
#define MAX_ARRS   0x10

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
    int *executor_sizep;
} command_type_t;

extern command_type_t vocabulary[];
extern int            vocabulary_size;

/* Define sandboxes for running programs */
typedef struct variable {
    char name[MAX_VAR_SZ];
    int value;
} var_t;

typedef struct array {
    char name[MAX_VAR_SZ];
    void *base;
} array_t;
typedef struct app_context {
    var_t vars[MAX_VARS];
    int n_vars;

    array_t arrays[MAX_ARRS];
    int n_arrays;

    int cur_code_idx;

    /* List of callbacks into the Engine */
    void *(*allocator)(sandbox_t *box, int size);
    void (*print_var)(sandbox_t *box, const char *var, int val);
} app_context_t;

typedef int (*app_executor_t)(const sandbox_t *box, app_context_t *ctx, int n_cmds);
typedef struct sandbox {
    command_t cmds[MAX_CMDS];
    
    void *cmd_code_ptrs[MAX_CMDS];
    int n_cmds;

    void *arena;        /* Generated code is put in the arena */
    int used_bytes;

    app_executor_t execute;
    app_context_t ctx;
} sandbox_t;
typedef char arena_t[ARENA_SIZE];
extern arena_t arenas[MAX_ARENAS];
extern int     n_arenas_used;

int engine_init();

int sandbox_init(sandbox_t *box);
int sandbox_add_command(sandbox_t *box, command_t cmd);
int sandbox_execute(sandbox_t *box, int n_cmds);

/* Callbacks from the webapp to the sandbox */
void *sandbox_alloc_trampoline(sandbox_t *box, int size);
void sandbox_print_var_trampoline(sandbox_t *box, const char *var, int val);

#endif /* ENGINE_H */