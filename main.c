#include "engine.h"
#include "external.h"
#include "util.h"

char program0[] = 
    "alloc arr0, 10;"
    "set arr0, 0, 0xdeadbeef;"
    "set arr0, 2, 5;"
    "set arr0, 4, 7;"
    "get arr0, 3, var0;"
    "get arr0, 4, var1;"
    "print var0;"
    "print var1;"
    "get arr0, 0, var0;"
;

char program1[] = 
    "alloc arr1, 10;"
    "get arr1, %d, mainsecret;"
    "print mainsecret;"
    "get arr1, %d, othersecret;"
    "print othersecret;"
;

int __attribute__((noreturn)) main() {
    char *cmdstr;
    command_t cmd;
    char *prog;
    long n_insts0, n_insts1;
    sandbox_t box0, box1;
    int ret;

    long secret = 0xdeadc0de;

    /* First, just initialize the engine */
    if((ret = engine_init()))
        goto error;

    /* Create the first sandbox, load with program0 
     * and execute it all.
     * Executing this program ensures that its secret has
     * been written to its data context */
    if(sandbox_init(&box0))
        goto error;
    prog = program0;
    n_insts0 = 0;
    while((cmdstr = command_next(&prog))) {
        n_insts0++;
        if(command_decode(cmdstr, &cmd) != 0)
            goto error;
        
        if(sandbox_add_command(&box0, cmd))
            goto error;
    }

    if(sandbox_execute(&box0, n_insts0))
        goto error;

    /* Spawn second sandbox, which tries to leak 
     * secrets from other sandboxes or secret above */
    if(sandbox_init(&box1))
        goto error;
    prog = program1;

    /* Decode and execute the first instruction to
     * allocate an array */
    cmdstr = command_next(&prog);
    if(command_decode(cmdstr, &cmd) != 0)
        goto error;
    if(sandbox_add_command(&box1, cmd))
        goto error;
    if(sandbox_execute(&box1, 1))
        goto error;

    /* Determine the offsets to the secrets, if required*/
    long main_secret_offset = 0;
    long other_secret_offset = 0;
#ifdef TRY_LEAK_MAIN_SECRET
    main_secret_offset = (long *)&secret - (long *)box1.ctx->arrays[0].base;
#endif
#ifdef TRY_LEAK_OTHER_PROG_SECRET
    other_secret_offset = (long *)box0.ctx->arrays[0].base - (long *)box1.ctx->arrays[0].base;
#endif
    char program_buf[256];
    uintptr_t args[] = {
        (uintptr_t)main_secret_offset,
        (uintptr_t)other_secret_offset
    };
    util_snprintf(program_buf, 256, prog, args);

    /* Execute the rest of the instructions in program1 */
    prog = program_buf;
    n_insts1 = 0;
    while((cmdstr = command_next(&prog))) {
        n_insts1++;
        if(command_decode(cmdstr, &cmd) != 0)
            goto error;
        
        if(sandbox_add_command(&box1, cmd))
            goto error;
    }

    if(sandbox_execute(&box1, n_insts1))
        goto error;

    goto success;

error:
    prints("Error\n", 6);
success:
    prints("Fini\n", 5);
    program_exit(ret);
}