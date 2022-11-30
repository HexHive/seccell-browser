

## Browser Architecture

The browser is built around an engine which can create sandboxes for webapps
to run.
A driver program (in main.c) uses the engine API to:

- create sandboxes for each webapp
- add commands for each webapp to its sandbox
- trigger the sandbox to run a few commands

The engine creates and maintains a `sandbox_t` structure for each sandbox.
For simplicity, we do not isolate this structure from the driver, but we 
assume that the driver will not directly modify this structure.
The sandbox also uses a dispatcher to run commands for each webapp.

The sandbox consists of:

- a code arena where the "compiled" code for the webapp is generated
- a `cmd_args_arena` where the commands and their arguments are stored
- a trampoline code arena, where the compartment switching trampoline is
  stored when the browser is compartmentalized
- the webapp's execution context
- function pointers for callbacks from the webapp

The webapp's context, in turn, consists of:
- a data arena where arrays can be allocated
- a list of arrays, tracking their storage space in the data arena
- a list of variables created by the webapp
- arguments read by the sandbox during callbacks

Commands are defined in the `commands.c` file, and require an executor and
(temporarily) a print funtion.
The engine needs to know the sizes of each executor function to be able to copy 
the executors into the sandbox code arenas, and we use the hacky 
`init_command_sizes` function.
We list the supported commands in a section below.

## Configuration options

Generic engine configuration options:

 - MAX_CMDS       - Max number of commands, storage in `sandbox_t`
 - MAX_CMD_SZ     - Max size of storage for command arguments in `sandbox_t`
 - MAX_SANDBOXES  - Max number of sandboxes supported, used by `scthreads`
 - ARENA_SIZE     - Max size of arenas: code arena, data arena, etc.
 - MAX_VARS       - Max number of variables stored in `app_context_t`
 - MAX_VAR_SZ     - Max size of name for variables and arrays
 - MAX_ARRS       - Max number of arrays stored in `app_context_t`

Engine compartmentalization configuration:

- CONFIG_COMP     - 1 for compartmentalized, 0 otherwise. For seL4, initialize
                    the sel4-playground build with -DEvaluationType=comp or
                    -DEvaluationType=uncomp
- CONFIG_SCTHREADS_STACK_SIZE - Stack size used for `scthreads`

Attacker configuration options:

There are four defines, trying to achieve four separate attacks, arbitrary 
reads/writes to the main program and another sandbox.
Comment out the defines for attacks which you want to try.
Note that the writes are not entirely independent, i.e., if any of the writes
is enabled in the compartmentalized version, the program will crash.

- TRY_LEAK_MAIN_SECRET
- TRY_LEAK_OTHER_PROG_SECRET
- TRY_CORRUPT_MAIN_SECRET
- TRY_CORRUPT_OTHER_PROG_SECRET

## Programming commands and syntax
```
prog -> prog stmt
prog -> stmt
stmt -> command arglist;
arglist -> arg
arglist -> arg arglist
arg -> var
arg -> num
```

`var` is any alphanumeric name
`num` is any number (in decimal or hexadecimal)

The engine currently supports the following commands:

- `alloc(arrname, #elements)` : Allocates an array of `#element` elements, with name `arrname`
- `set arrname, idx, val`: Sets element at index `idx` in array `arrname` to value `val`
- `get arrname, idx, varname`: Sets the value of variable `varname` to value in array `arrname`
                                at index `idx`. Variable is created if not existing
- `print varname`: Print the variable `varname`

## Code generation by copying

While the Engine "generates" code for sandboxes, this does not involve the 
traditional compiler-like architecture-specific code-generating backends.
Instead, the engine copies over pre-compiled interpreter functions.
This approach leads to one of the most important requirements when adding
commands and their executors: the executors must be "pure".

A pure function is one whose execution depends entirely on its arguments,
and which does not call any other functions.
Both requirements stem from the same cause: globals and other functions
are addressed using instruction pointer-relative offsets. 
When code is copied to a different address, generated code and data 
pointers will be invalid, and we don't want to complicate the Engine
by finding and modifying these pointer generation code.
Instead, when executors need to call external functions, specially
for callbacks into the Engine, we rely on function pointers stored
in either the `sandbox_t` and pass callback arguments through `app_context_t`.
