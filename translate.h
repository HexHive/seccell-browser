
typedef struct command {
    char *opcode;
    void (*print)(char *arg0, char *arg1, char *arg2);
} command_t;

extern command_t commands[];
extern int       n_commands;

char *next_command(char **prog);
int interpret_command(char *cmd, char **arg0, char **arg1, char **arg2);
int match(char *opcode, int idx);
