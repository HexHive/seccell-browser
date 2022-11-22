#include "translate.h"
#include <stdio.h>

char program[] = 
    "alloc array, 10;"
    "set array, 0, 1;"
    "set array, 2, 5;"
    "set array, 4, 7;"
    "get array, 5;"
    "get array, 7;";

int main() {
    char *cmd, *arg0, *arg1, *arg2;
    char *prog = program;

    while((cmd = next_command(&prog))) {
        printf("%s\n", cmd);
        if(interpret_command(cmd, &arg0, &arg1, &arg2) != 0)
            break;
        
        printf("cmd: %s arg0: %s\n", cmd, arg0);

        for(int j = 0; j < n_commands; j++)
            if(match(cmd, j))
                commands[j].print(arg0, arg1, arg2);
    }
}