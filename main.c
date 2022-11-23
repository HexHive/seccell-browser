#include "translate.h"
#include <stdio.h>

char program0[] = 
    "alloc arr, 10;"
    "set arr, 0, 1;"
    "set arr, 2, 5;"
    "set arr, 4, 7;"
    "get arr, 5;"
    "get arr, 7;";

int main() {
    char *cmdstr;
    command_t cmd;
    char *prog = program0;

    while((cmdstr = command_next(&prog))) {
        printf("%s\n", cmdstr);
        if(command_decode(cmdstr, &cmd) != 0)
            break;
        
        for(int j = 0; j < vocabulary_size; j++)
            if(command_match(cmd, j))
                vocabulary[j].print(cmd);
    }
}