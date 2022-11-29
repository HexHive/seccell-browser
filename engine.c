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
void    *contexts;
int engine_init() {
    if(platform_specific_setup(&contexts))
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

static void *__sandbox_alloc(char *reg, long *reg_used_bytes, long size) {
    reg += *reg_used_bytes;

    if(*reg_used_bytes + size > ARENA_SIZE)
        return NULL;
    *reg_used_bytes += size;

    return reg;
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

    return __sandbox_alloc(reg, reg_used_bytes, size);
}

static void *sandbox_alloc_data(sandbox_t *box) {
    long size = box->ctx->alloc_size;
    void *space = sandbox_alloc(box, size, 0);
#if CONFIG_COMP
    box->trampoline_return(box->contexts, space);
#else
    return space;
#endif
}

static void *sandbox_print_var(const sandbox_t *box) {
    char buf[256];
    const char *varname = box->ctx->print_var;
    long varvalue = box->ctx->print_val;
    uintptr_t args[] = {
        (uintptr_t)varname, 
        (uintptr_t)varvalue
    };
    long size = util_snprintf(buf, 256, "%s: %x\n", args);
    prints(buf, size);

#if CONFIG_COMP
    box->trampoline_return(box->contexts, NULL);
#else
    return NULL;
#endif
}

sandbox_t *sandbox_alloc_init() {
    sandbox_t *box = mmap_region(NULL, sizeof(sandbox_t), 1, 1, 0);

    if(box) {
        if(sandbox_init(box))
            return NULL;
    }
    
    return box;
}

int sandbox_init(sandbox_t *box) {
    if(n_arenas_used >= MAX_SANDBOXES)
        return -1;
    
    /* Allocating the sandbox an unused, empty arena */
    box->carena = alloc_arena();
    box->c_used_bytes = 0;
    box->trampoline_carena = alloc_arena();
    box->t_carena_used_bytes = 0;
    box->cmd_args_used_bytes = 0;
    box->ctx = alloc_ctx();
    box->allocator = (trampoline_fn_t)sandbox_alloc_data;
    box->print_var = sandbox_print_var;
    box->n_cmds = 0;
    box->execute = sandbox_alloc(box, execute_commands_size, 1);

    /* Set up context, including pointers to copied trampolines */
    box->ctx->n_arrays = 0;
    box->ctx->n_vars   = 0;
    box->ctx->cur_code_idx = 0;

    /* Copy executor into the code arena, trampolines into trampoline arena */
    util_memcpy((void *)box->execute, (void *)execute_commands, execute_commands_size);

#ifdef CONFIG_DEBUG
    box->execute = execute_commands;
#endif

#if CONFIG_COMP
    // box->comp_id = allocate_compartment();
    //TODO: Make secdiv allocation within allocate_compartment 
    // instead of scthreads_init_contexts
    box->comp_id = n_arenas_used + 2;

    int trampoline_len = (long)(uintptr_t)scthreads_end - (long)(uintptr_t)scthreads_start;
    void *space = __sandbox_alloc(box->trampoline_carena, 
                                            &box->t_carena_used_bytes,
                                            trampoline_len);
    util_memcpy(space, (void *)scthreads_start, trampoline_len);
    int scthreads_call_offset = (char *)scthreads_call - (char *)scthreads_start;
    box->trampoline_call = (void *)((char *)space + scthreads_call_offset);
    int scthreads_ret_offset = (char *)scthreads_return - (char *)scthreads_start;
    box->trampoline_return = (void *)((char *)space + scthreads_ret_offset);

    box->contexts = contexts;

    /* Compartment gets:
     * 1. rx permission for carena  
     * 2. rx permission for trampoline arena 
     * 3. rw permission for ctx
     * 4. ro permission for sandbox                
     **/
    compartment_permit(box->comp_id, box->carena, 1, 0, 1);
    compartment_permit(box->comp_id, box->trampoline_carena, 1, 0, 1);
    compartment_permit(box->comp_id, box->ctx, 1, 1, 0);
    compartment_permit(box->comp_id, box, 1, 0, 0);
    /* Engine drops to:
     * 1. rw permission on carena 
     * 2. rx permission on trampoline arena
     **/
    protect_region(box->carena, ARENA_SIZE, 1, 1, 0);
    protect_region(box->trampoline_carena, ARENA_SIZE, 1, 0, 1);
#endif
    
    n_arenas_used++;
    return 0;
}

static void copy_command(sandbox_t *box, command_t *dst, command_t src) {
    void *space;
    int arg_len;
    
    util_memset((char *)dst->args, 0, MAX_ARGS * sizeof(dst->args[0]));
    for(int i = 0; i < MAX_ARGS; i++) {
        char *arg = src.args[i];
        if(!arg)
            break;

        arg_len = util_strlen(arg) + 1;
        space = __sandbox_alloc(box->cmd_args_arena, 
                                &box->cmd_args_used_bytes, 
                                arg_len);
        dst->args[i] = space;
        util_strcpy(dst->args[i], src.args[i]);
    }
}

int sandbox_add_command(sandbox_t *box, command_t cmd) {
    long cmd_idx = box->n_cmds;

    if(cmd_idx >= MAX_CMDS)
        return -1;

    copy_command(box, &box->cmds[cmd_idx], cmd);
    for(long j = 0; j < vocabulary_size; j++)
        if(command_match(cmd, j)) {
            /* Copy code for the command into the arena and set the 
             * executor accordingly */
            long executor_size = *vocabulary[j].executor_sizep;
            void *space = sandbox_alloc(box,  executor_size, 1);
            if(!space)
                return -1;
            util_memcpy(space, (void *)vocabulary[j].execute, executor_size);
#ifdef CONFIG_DEBUG
            box->cmds[cmd_idx].execute = vocabulary[j].execute;
#else
            box->cmds[cmd_idx].execute = space;
#endif
            break;
        }

    box->n_cmds++;
    return 0;
}

int sandbox_execute(sandbox_t *box, long n_cmds) {
    box->ctx->n_cmds = n_cmds;
#if CONFIG_COMP
    return (int)(uintptr_t)box->trampoline_call(contexts, box->comp_id, 
                                    (trampoline_fn_t)box->execute, box);
#else
    return box->execute(box);
#endif
}
