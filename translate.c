#include "translate.h"
#include "util.h"
#include <stdio.h>

void print_alloc(char **args) {
    printf("allocate array named %s, size %d\n", args[1], util_strtod(args[2]));
}

void print_get(char **args) {
    printf("get array %s index %d\n", args[1], util_strtod(args[2]));
}

void print_set(char **args) {
    printf("set array %s index %d to %d\n", args[1], util_strtod(args[2]), util_strtod(args[3]));
}

command_type_t vocabulary[] = {
    { .opcode = "alloc", .print = print_alloc },
    { .opcode = "get",   .print = print_get   },
    { .opcode = "set",   .print = print_set   }
};
int vocabulary_size = sizeof(vocabulary) / sizeof(vocabulary[0]);

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

int command_interpret(char *cmd, char **args) {
    char c, prevc = 0;
    int state = 0;
    
    for(int j = 0; j < MAX_ARGS; j++)
        args[j] = NULL;

    while (1) {
        c = *cmd;

        if (state < MAX_ARGS) {
            if(!is_separator(c) && is_separator(prevc))
                
                args[state] = cmd;
            else if(!is_separator(prevc) && is_separator(c)) {
                *cmd = '\0';
                state++;
            }
        } else 
            return -1;

        if(!c)
            break;

        prevc = c;
        cmd++;
    }
    return 0;
}

int command_match(char *opcode, int idx) {
    if(idx >= vocabulary_size)
        return 0;

    char *cmd_opcode = vocabulary[idx].opcode;

    return util_strcmp(opcode, cmd_opcode) == 0;
}
