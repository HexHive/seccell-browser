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

long     n_arenas_used = 0;
int engine_init() {
    if(platform_specific_setup())
        return 1;

    init_command_sizes();
    n_arenas_used = 0;

    return 0;
}
static void *alloc_arena() {
    return mmap_region(NULL, ARENA_SIZE, 1, 1, 1);;
}
static void *alloc_ctx() {
    return mmap_region(NULL, sizeof(app_context_t), 1, 1, 0);;
}

// TODO: Possibly allocate illegal instructions between the 
// existing allocation and the new one, in order to catch some
// bugs
static void *sandbox_alloc(sandbox_t *box, long size, int code) {
    char *reg;
    long *reg_used_bytes;

    if(code) {
        reg = box->carena;
        reg_used_bytes = &box->c_used_bytes;
    } else {
        reg = &box->ctx->darena[0];
        reg_used_bytes = &box->ctx->d_used_bytes;
    }
    reg += *reg_used_bytes;

    if(*reg_used_bytes + size > ARENA_SIZE)
        return NULL;
    *reg_used_bytes += size;

    return reg;
}

int sandbox_init(sandbox_t *box) {
    if(n_arenas_used >= MAX_SANDBOXES)
        return -1;
    
    /* Allocating the sandbox an unused, empty arena */
    box->carena = alloc_arena();
    box->ctx = alloc_ctx();

    box->n_cmds = 0;
    box->c_used_bytes = 0;

    box->ctx->n_arrays = 0;
    box->ctx->n_vars   = 0;
    box->ctx->cur_code_idx = 0;
    box->ctx->allocator = sandbox_alloc_trampoline;
    box->ctx->print_var = sandbox_print_var_trampoline;

    /* Copy executor into the code arena */
    void *space = sandbox_alloc(box, execute_commands_size, 1);
    util_memcpy(space, (void *)execute_commands, execute_commands_size);
#ifdef DEBUG
    box->execute = execute_commands;
#else
    box->execute = space;
#endif
    
    n_arenas_used++;
    return 0;
}

int sandbox_add_command(sandbox_t *box, command_t cmd) {
    long cmd_idx = box->n_cmds;

    if(cmd_idx >= MAX_CMDS)
        return -1;

    box->cmds[cmd_idx] = cmd;
    for(long j = 0; j < vocabulary_size; j++)
        if(command_match(cmd, j)) {
            /* Copy code for the command into the arena and set the 
             * executor accordingly */
            long executor_size = *vocabulary[j].executor_sizep;
            void *space = sandbox_alloc(box,  executor_size, 1);
            if(!space)
                return -1;
            util_memcpy(space, (void *)vocabulary[j].execute, executor_size);
#ifdef DEBUG
            box->cmds[cmd_idx].execute = vocabulary[j].execute;
#else
            box->cmds[cmd_idx].execute = space;
#endif
        }

    box->n_cmds++;
    return 0;
}

int sandbox_execute(sandbox_t *box, long n_cmds) {
    return box->execute(box, box->ctx, n_cmds);
}

void *sandbox_alloc_trampoline(sandbox_t *box, long size) {
    // TODO: Implement compartment switching here

    return sandbox_alloc(box, size, 0);
}

void sandbox_print_var(sandbox_t *box, const char *varname, long varvalue) {
    char buf[256];
    uintptr_t args[] = {
        (uintptr_t)varname, 
        (uintptr_t)varvalue
    };
    long size = util_snprintf(buf, 256, "%s: %x\n", args);
    prints(buf, size);
}

void sandbox_print_var_trampoline(sandbox_t *box, const char *var, long val) {
    // TODO: Implement compartment switching here

    sandbox_print_var(box, var, val);
}