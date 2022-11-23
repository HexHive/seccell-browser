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
    char *cmd, *args[MAX_ARGS];
    char *prog = program0;

    while((cmd = command_next(&prog))) {
        printf("%s\n", cmd);
        if(command_interpret(cmd, args) != 0)
            break;
        
        for(int j = 0; j < vocabulary_size; j++)
            if(command_match(cmd, j))
                vocabulary[j].print(args);
    }
}