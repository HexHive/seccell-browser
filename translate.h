
#define MAX_ARGS   (1 + 4)
#define MAX_CMDS   0x100
#define MAX_ARENAS 0x10
#define ARENA_SIZE 0x1000

/* Defines individual commands */
typedef struct command {
    char *args[MAX_ARGS];     /* args[0] is the opcode */
} command_t;

char *command_next(char **prog);
int command_decode(char *cmdstr, command_t *cmd);
int command_match(command_t cmd, int idx);

/* Defines types of valid commands */
typedef struct command_type {
    char *opcode;
    int n_args;
    void (*print)(command_t cmd);
} command_type_t;

extern command_type_t vocabulary[];
extern int            vocabulary_size;

/* Define sandboxes for running programs */
typedef struct sandbox {
    command_t cmds[MAX_CMDS];
    int       cmd_code_idx[MAX_CMDS];
    int n_cmds;
    void *arena;        /* Generated code is put in the arena */

    int cur_code_idx;
} sandbox_t;
typedef char arena_t[ARENA_SIZE];
extern arena_t arenas[MAX_ARENAS];
extern int     n_arenas_used;

int sandbox_init(sandbox_t *box);
int sandbox_add_command(sandbox_t *box, command_t cmd);
void sandbox_execute();
