#include "translate.h"
#include <stdio.h>

char program0[] = 
    "alloc arr, 10;"
    "set arr, 0, 1;"
    "set arr, 2, 5;"
    "set arr, 4, 7;"
    "get arr, 3, var0;"
    "get arr, 4, var1;"
    "print var0;"
    "print var1;"
;

int main() {
    char *cmdstr;
    command_t cmd;
    char *prog = program0;
    int n_insts0 = 0;
    sandbox_t box0;

    sandbox_init(&box0);
    while((cmdstr = command_next(&prog))) {
        n_insts0++;
        if(command_decode(cmdstr, &cmd) != 0)
            break;
        
        // for(int j = 0; j < vocabulary_size; j++)
        //     if(command_match(cmd, j))
        //         vocabulary[j].print(cmd);
                
        sandbox_add_command(&box0, cmd);
    }

    sandbox_execute(&box0, n_insts0);
}