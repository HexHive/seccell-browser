#include "commands.h"
#include "external.h"
#include "engine.h"
#include "util.h"

int execute_commands_size;
int alloc_executor_size;
int get_executor_size;
int set_executor_size;
int print_executor_size;
void init_command_sizes() {
    /* It does not matter if these sizes are not exact. 
     * They need to be at least as big as the actual size, bigger is ok */
    execute_commands_size = (int)(uintptr_t)alloc_executor - (int)(uintptr_t)execute_commands;
    alloc_executor_size   = (int)(uintptr_t)get_executor   - (int)(uintptr_t)alloc_executor  ;
    get_executor_size     = (int)(uintptr_t)set_executor   - (int)(uintptr_t)get_executor    ;
    set_executor_size     = (int)(uintptr_t)print_executor - (int)(uintptr_t)set_executor    ;
    print_executor_size   = (int)(uintptr_t)print_alloc    - (int)(uintptr_t)print_executor  ;
}

/* Returns the number of commands not executed */
int execute_commands(const sandbox_t *box, app_context_t *ctx, int n_cmds) {
    command_executor_t executor;
    const command_t *cmd;
    int ret;

    while(n_cmds > 0) {
        if(ctx->cur_code_idx > box->n_cmds)
            return -1;

        cmd = &box->cmds[ctx->cur_code_idx];
        ret = cmd->execute(box, ctx, cmd);

        if(ret != 0)
            return n_cmds;

        n_cmds--;
        ctx->cur_code_idx++;
    }
    return 0;
}

int alloc_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd) {
    const char *arrname = cmd->args[1];
    int arrsize = util_strtod(cmd->args[2]);

    if(ctx->n_arrays >= MAX_ARRS)
        return -1;

    for(int i = 0; i < ctx->n_arrays; i++)
        if(util_strcmp(ctx->arrays[i].name, arrname) == 0)
            return -1;

    int *alloc_base = ctx->allocator((sandbox_t *)box, arrsize * sizeof(int));

    util_strcpy(ctx->arrays[ctx->n_arrays].name, arrname);
    ctx->arrays[ctx->n_arrays].base = alloc_base;

    ctx->n_arrays++;
    return 0;
}

int get_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd) {
    const char *arrname = cmd->args[1];
    int idx = util_strtod(cmd->args[2]);
    const char *varname = cmd->args[3];

    if(!arrname || !varname)
        return -1;

    int *arrbase = NULL;
    for(int i = 0; i < ctx->n_arrays; i++)
        if(util_strcmp(ctx->arrays[i].name, arrname) == 0) {
            arrbase = ctx->arrays[i].base;
            break;
        }
    if(!arrbase)
        return -1;

    int *varvalue = NULL;
    for(int i = 0; i < ctx->n_vars; i++)
        if(util_strcmp(ctx->vars[i].name, varname) == 0) {
            varvalue = &ctx->vars[i].value;
            break;
        }
    if(!varvalue) {
        if(ctx->n_vars >= MAX_VARS)
            return -1;

        var_t *var = &ctx->vars[ctx->n_vars];
        util_strcpy(var->name, varname);
        varvalue = &var->value;
        ctx->n_vars++;
    }

    /* Note: unchecked array index here allows out-of-bounds read */
    *varvalue = arrbase[idx];
    return 0;
}

int set_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd) {
    const char *arrname = cmd->args[1];
    int idx = util_strtod(cmd->args[2]);
    int value = util_strtod(cmd->args[3]);

    if(!arrname)
        return -1;

    int *arrbase = NULL;
    for(int i = 0; i < ctx->n_arrays; i++)
        if(util_strcmp(ctx->arrays[i].name, arrname) == 0) {
            arrbase = ctx->arrays[i].base;
            break;
        }
    if(!arrbase)
        return -1;

    /* Note: unchecked array index here allows out-of-bounds write */
    arrbase[idx] = value;

    return 0;
}

int print_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd) {
    const char *varname = cmd->args[1];

    if(!varname)
        return -1;

    int *varvalue = NULL;
    for(int i = 0; i < ctx->n_vars; i++)
        if(util_strcmp(ctx->vars[i].name, varname) == 0) {
            varvalue = &ctx->vars[i].value;
            break;
        }
    if(!varvalue)
        return -1;

    ctx->print_var((sandbox_t *)box, varname, *varvalue);

    return 0;
}

void print_alloc(command_t cmd) {
    char buf[256];
    uintptr_t args[] = {
        (uintptr_t)cmd.args[1], 
        (uintptr_t)util_strtod(cmd.args[2])
    };
    int size = util_snprintf(buf, 256, "allocate array named %s, size %d\n", args);
    prints(buf, size);
}

void print_get(command_t cmd) {
    char buf[256];
    uintptr_t args[] = {
        (uintptr_t)cmd.args[1], 
        (uintptr_t)util_strtod(cmd.args[2]), 
        (uintptr_t)cmd.args[3]
    };
    int size = util_snprintf(buf, 256, "read array %s index %d into variable %s\n", args);
    prints(buf, size);
}

void print_set(command_t cmd) {
    char buf[256];
    uintptr_t args[] = {
        (uintptr_t)cmd.args[1], 
        (uintptr_t)util_strtod(cmd.args[2]), 
        (uintptr_t)util_strtod(cmd.args[3])
    };
    int size = util_snprintf(buf, 256, "set array %s index %d to %d\n", args);
    prints(buf, size);
}

void print_print(command_t cmd) {
    char buf[256];
    uintptr_t args[] = {
        (uintptr_t)cmd.args[1]
    };
    int size = util_snprintf(buf, 256, "print variable %s\n", args);
    prints(buf, size);
}