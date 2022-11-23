#include "translate.h"
#include "util.h"
#include <stdio.h>

char *command_next(char **prog) {
    char *start, *end, c;
    start = end = *prog;

    while ((c = *end) != ';') { 
        if(!c)
            return NULL;
        end++;
    }
    
    *end = '\0';
    *prog = end + 1;

    return start;
}

static int is_separator(char c) {
    return c == ' ' || c == ',' || c == '\0';
}

int command_decode(char *cmdstr, command_t *cmd) {
    char c, prevc = 0;
    int state = 0;
    
    for(int j = 0; j < MAX_ARGS; j++)
        cmd->args[j] = NULL;

    while (1) {
        c = *cmdstr;

        if (state < MAX_ARGS) {
            if(!is_separator(c) && is_separator(prevc))
                
                cmd->args[state] = cmdstr;
            else if(!is_separator(prevc) && is_separator(c)) {
                *cmdstr = '\0';
                state++;
            }
        } else 
            return -1;

        if(!c)
            break;

        prevc = c;
        cmdstr++;
    }
    return 0;
}

int command_match(command_t cmd, int idx) {
    char *opcode = cmd.args[0];

    if(idx >= vocabulary_size)
        return 0;

    char *cmd_opcode = vocabulary[idx].opcode;

    return util_strcmp(opcode, cmd_opcode) == 0;
}

static void print_alloc(command_t cmd) {
    printf("allocate array named %s, size %d\n", cmd.args[1], util_strtod(cmd.args[2]));
}

static void print_get(command_t cmd) {
    printf("read array %s index %d into variable %s\n", cmd.args[1], util_strtod(cmd.args[2]), cmd.args[3]);
}

static void print_set(command_t cmd) {
    printf("set array %s index %d to %d\n", cmd.args[1], util_strtod(cmd.args[2]), util_strtod(cmd.args[3]));
}

static void print_print(command_t cmd) {
    printf("print variable %s\n", cmd.args[1]);
}

command_type_t vocabulary[] = {
    { .opcode = "alloc", .n_args = 3, .print = print_alloc },
    { .opcode = "get",   .n_args = 4, .print = print_get   },
    { .opcode = "set",   .n_args = 4, .print = print_set   },
    { .opcode = "print", .n_args = 2, .print = print_print }
};
int vocabulary_size = sizeof(vocabulary) / sizeof(vocabulary[0]);

arena_t arenas[MAX_ARENAS] __aligned__(ARENA_SIZE);
int     n_arenas_used;

int sandbox_init(sandbox_t *box) {
    if(n_arenas_used >= MAX_ARENAS)
        return -1;
    
    box->n_cmds = 0;
    box->arena = arenas[n_arenas_used];
    n_arenas_used++;
    mprotect(box->arena, ARENA_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);

    box->cur_code_idx = 0;

    return 0;
}

int sandbox_add_command(sandbox_t *box, command_t cmd) {
    box->cmds[box->n_cmds++] = cmd;
    /* TODO: Also translate to arena here */

    return 0;
}
void sandbox_execute() {

}
