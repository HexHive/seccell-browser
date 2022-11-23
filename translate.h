
#define MAX_ARGS (1 + 4)

/* Defines types of valid commands */
typedef struct command_type {
    char *opcode;
    void (*print)(char **args);
} command_type_t;

extern command_type_t vocabulary[];
extern int            vocabulary_size;

/* Defines individual commands */
typedef struct command {
    char *args[MAX_ARGS];     /* args[0] is the opcode */
} command_t;

char *command_next(char **prog);
int command_interpret(char *cmd, char **args);
int command_match(char *opcode, int idx);
