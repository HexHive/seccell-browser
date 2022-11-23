#ifndef COMMAND_H
#define COMMAND_H

#include "translate.h"

void init_command_sizes();

int execute_commands(const sandbox_t *box, app_context_t *ctx, int n_cmds);
extern int execute_commands_size;

int alloc_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
int get_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
int set_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
int print_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
extern int alloc_executor_size;
extern int get_executor_size;
extern int set_executor_size;
extern int print_executor_size;

void print_alloc(command_t cmd);
void print_get(command_t cmd);
void print_set(command_t cmd);
void print_print(command_t cmd);

#endif /* COMMAND_H */