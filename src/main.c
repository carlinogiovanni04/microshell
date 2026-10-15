#include "base.h"

#if defined(MICROSHELL_PROFILE)
#define MICROPROF_IMPLEMENTATION
#include "microprof.h"
#else
/* Instrumentation compiles away when profiling is not enabled. */
#define PROF_SCOPE(name)
#define PROF_FUNCTION()
#define prof_frame_end() ((void)0)
#endif

#define MAX_INPUT_SIZE (u64)256

void cmd_exit(const char* args, b32* running);
void cmd_print(const char* args, b32* running);
void cmd_help(const char* args, b32* running);

typedef void (*CommandFunc)(const char* args, b32* running);

typedef struct {
    const char* name;
    CommandFunc func;
    const char* description;
} Command;

void cmd_exit(const char* args, b32* running) {
    PROF_FUNCTION() {
        (void)args;
        *running = false;
    }
}

void cmd_print(const char* args, b32* running) {
    PROF_FUNCTION() {
        (void)running;
        base_cstr_printf("%s\n", args);
    }
}

static void ls_print_entry(const DirEntry* e, void* user) {
    PROF_FUNCTION() {
        (void)user;

        base_cstr_printf("%d %s\n", (i32)e->size, e->name);
    }
}

void cmd_ls(const char *args, b32 *running)
{
  PROF_FUNCTION() {
      (void)running;
      const char *path = (args[0] != '\0') ? args : ".";
      if (!sys_dir_foreach(path, ls_print_entry, 0)) {
          base_cstr_printf("ls: cannot open '%s'\n", path);
      }
  }
}

void cmd_help(const char* args, b32* running) {
    PROF_FUNCTION() {
        (void)args;
        (void)running;
        base_cstr_printf("Available commands:\n");
        base_cstr_printf("  print [text] - Print text to the screen\n");
        base_cstr_printf("  ls           - List files and directories\n");
        base_cstr_printf("  help         - Display this help menu\n");
        base_cstr_printf("  exit         - Exit the shell\n");
    }
}

Command commands[] = {{"exit", cmd_exit, "Exits the shell"},
                      {"print", cmd_print, "Prints arguments"},
                      {"help", cmd_help, "Shows help menu"},
                      {"ls", cmd_ls, "Lists file and directories"}};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

int main(void) {
    b32 running = true;
    char input[MAX_INPUT_SIZE];

    while (running) {
        PROF_SCOPE("shell.frame") {
            base_cstr_printf("> ");

            PROF_SCOPE("shell.read") {
                base_cstr_readline(input, MAX_INPUT_SIZE);
            }

            if (input[0] == '\0') {
                continue;
            }

            char* cmd_name = input;
            char* args = input;

            PROF_SCOPE("shell.parse") {
                while (*args != '\0' && *args != ' ' && *args != '\t') {
                    args++;
                }

                if (*args != '\0') {
                    *args = '\0'; 
                    args++;       // Move past the space
                    while (*args == ' ' || *args == '\t') {
                        args++;   
                    }
                }
            }

            b32 found = false;
            PROF_SCOPE("shell.dispatch") {
                for (usize i = 0; i < COMMAND_COUNT; i++) {
                    if (base_cstr_compare(cmd_name, commands[i].name) == 0) {
                        commands[i].func(args, &running);
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                base_cstr_printf("Unknown command: %s. Type 'help' for a list of commands.\n", cmd_name);
            }
        }
        prof_frame_end();
    }

#if defined(MICROSHELL_PROFILE)
    prof_report();
    prof_write_trace("microshell_trace.json");
#endif

    return 0;
}
