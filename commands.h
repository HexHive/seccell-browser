#ifndef COMMAND_H
#define COMMAND_H

#include "engine.h"

void init_command_sizes();

/* Command dispatcher */
int execute_commands(const sandbox_t *box);
extern long execute_commands_size;
/* Command executors */
int alloc_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
int get_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
int set_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
int print_executor(const sandbox_t *box, app_context_t *ctx, const command_t *cmd);
extern long alloc_executor_size;
extern long get_executor_size;
extern long set_executor_size;
extern long print_executor_size;
/* Command printers */
void print_alloc(command_t cmd);
void print_get(command_t cmd);
void print_set(command_t cmd);
void print_print(command_t cmd);

#endif /* COMMAND_H */