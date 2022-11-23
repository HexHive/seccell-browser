#include "commands.h"
#include "external.h"
#include "engine.h"
#include "util.h"

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

command_type_t vocabulary[] = {
    { .opcode = "alloc", .n_args = 3, .print = print_alloc, .execute = alloc_executor, .executor_sizep = &alloc_executor_size },
    { .opcode = "get",   .n_args = 4, .print = print_get,   .execute = get_executor,   .executor_sizep = &get_executor_size },
    { .opcode = "set",   .n_args = 4, .print = print_set,   .execute = set_executor,   .executor_sizep = &set_executor_size },
    { .opcode = "print", .n_args = 2, .print = print_print, .execute = print_executor, .executor_sizep = &print_executor_size }
};
int vocabulary_size = sizeof(vocabulary) / sizeof(vocabulary[0]);

arena_t arenas[MAX_ARENAS] __attribute__((aligned(ARENA_SIZE)));
int     n_arenas_used;

// TODO: Possibly allocate illegal instructions between the 
// existing allocation and the new one, in order to catch some
// bugs
static void *sandbox_alloc(sandbox_t *box, int size) {
    char *reg = box->arena + box->used_bytes;
    if(box->used_bytes + size > ARENA_SIZE)
        return NULL;
    
    box->used_bytes += size;
    return reg;
}

int sandbox_init(sandbox_t *box) {
    if(n_arenas_used >= MAX_ARENAS)
        return -1;
    
    /* Allocating the sandbox an unused, empty arena */
    box->n_cmds = 0;
    box->arena = arenas[n_arenas_used];
    n_arenas_used++;
    box->used_bytes = 0;
    protect_region(box->arena, ARENA_SIZE, 1, 1, 1);

    init_command_sizes();

    /* Copy executor into the arena */
    void *space = sandbox_alloc(box, execute_commands_size);
    util_memcpy(space, (void *)execute_commands, execute_commands_size);

    box->ctx.n_arrays = 0;
    box->ctx.n_vars   = 0;
    box->ctx.cur_code_idx = 0;
    box->ctx.allocator = sandbox_alloc_trampoline;
    box->ctx.print_var = sandbox_print_var_trampoline;

    box->execute = space;
    // box->execute = execute_commands;

    return 0;
}

int sandbox_add_command(sandbox_t *box, command_t cmd) {
    int cmd_idx = box->n_cmds;

    if(cmd_idx >= MAX_CMDS)
        return -1;

    box->cmds[cmd_idx] = cmd;
    for(int j = 0; j < vocabulary_size; j++)
        if(command_match(cmd, j)) {
            /* Copy code for the command into the arena and set the 
             * executor accordingly */
            int executor_size = *vocabulary[j].executor_sizep;
            void *space = sandbox_alloc(box,  executor_size);
            if(!space)
                return -1;
            util_memcpy(space, (void *)vocabulary[j].execute, executor_size);
            box->cmds[cmd_idx].execute = space;
        }

    box->n_cmds++;
    return 0;
}

int sandbox_execute(sandbox_t *box, int n_cmds) {
    return box->execute(box, &box->ctx, n_cmds);
}

void *sandbox_alloc_trampoline(sandbox_t *box, int size) {
    // TODO: Implement compartment switching here

    return sandbox_alloc(box, size);
}

void sandbox_print_var(sandbox_t *box, const char *varname, int varvalue) {
    char buf[256];
    uintptr_t args[] = {
        (uintptr_t)varname, 
        (uintptr_t)varvalue
    };
    int size = util_snprintf(buf, 256, "%s: %d\n", args);
    prints(buf, size);
}

void sandbox_print_var_trampoline(sandbox_t *box, const char *var, int val) {
    // TODO: Implement compartment switching here

    sandbox_print_var(box, var, val);
}