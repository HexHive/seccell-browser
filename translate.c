#include "translate.h"
#include "util.h"
#include <stdio.h>

void print_alloc(char *arg0, char *arg1, char *arg2) {
    printf("allocate array named %s, size %d\n", arg0, strtod(arg1));
}

void print_get(char *arg0, char *arg1, char *arg2) {
    printf("get array %s index %d\n", arg0, strtod(arg1));
}

void print_set(char *arg0, char *arg1, char *arg2) {
    printf("set array %s index %d to %d\n", arg0, strtod(arg1), strtod(arg2));
}

command_t commands[] = {
    { .opcode = "alloc", .print = print_alloc },
    { .opcode = "get",   .print = print_get   },
    { .opcode = "set",   .print = print_set   }
};
int n_commands = sizeof(commands) / sizeof(commands[0]);

char *next_command(char **prog) {
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

int interpret_command(char *cmd, char **arg0, char **arg1, char **arg2) {
    char c, prevc = 0;
    int state = 0;
    
    *arg0 = *arg1 = *arg2 = NULL;
    while (1) {
        c = *cmd;

        if(state == 0) {
            if(prevc != ' ' && c == ' ') {
                *cmd = '\0';
                state = 1;
            }
        } else if(state == 1) {
            if(c != ' ' && (prevc == ' ' || prevc == ','))
                *arg0 = cmd;
            else if((prevc != ' ' && prevc != ',') && (c == ' ' || c == ',' || c == '\0')) {
                *cmd = '\0';
                state = 2;
            }
        } else if(state == 2) {
            if(c != ' ' && (prevc == ' ' || prevc == ','))
                *arg1 = cmd;
            else if((prevc != ' ' && prevc != ',') && (c == ' ' || c == ',' || c == '\0')) {
                *cmd = '\0';
                state = 3;
            }
        } else if(state == 3) {
            if(c != ' ' && (prevc == ' ' || prevc == ','))
                *arg2 = cmd;
            else if((prevc != ' ' && prevc != ',') && (c == ' ' || c == ',' || c == '\0')) {
                *cmd = '\0';
                break;
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

int match(char *opcode, int idx) {
    if(idx >= n_commands)
        return 0;

    char *cmd_opcode = commands[idx].opcode;

    return simple_strcmp(opcode, cmd_opcode) == 0;
}
