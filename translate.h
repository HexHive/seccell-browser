
#define MAX_ARGS (1 + 4)

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
