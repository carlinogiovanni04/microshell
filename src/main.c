#include "base.h"

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
    (void)args;
    *running = false;
}

void cmd_print(const char* args, b32* running) {
    (void)running;
    base_cstr_printf("%s\n", args);
}

static void ls_print_entry(const DirEntry* e, void* user) {
    (void)user;

    base_cstr_printf("%d %s\n", (i32)e->size, e->name);
}

void cmd_ls(const char *args, b32 *running)
{
  (void)running;
  const char *path = (args[0] != '\0') ? args : ".";
  if (!sys_dir_foreach(path, ls_print_entry, 0)) {
      base_cstr_printf("ls: cannot open '%s'\n", path);
  }
}

void cmd_help(const char* args, b32* running) {
    (void)args;
    (void)running;
    base_cstr_printf("Available commands:\n");
    base_cstr_printf("  print [text] - Print text to the screen\n");
    base_cstr_printf("  ls           - List files and directories\n");
    base_cstr_printf("  help         - Display this help menu\n");
    base_cstr_printf("  exit         - Exit the shell\n");
    
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
        base_cstr_printf("> ");
        base_cstr_readline(input, MAX_INPUT_SIZE);

        if (input[0] == '\0') {
            continue;
        }

        char* cmd_name = input;
        char* args = input;

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

        b32 found = false;
        for (usize i = 0; i < COMMAND_COUNT; i++) {
            if (base_cstr_compare(cmd_name, commands[i].name) == 0) {
                commands[i].func(args, &running);
                found = true;
                break;
            }
        }

        if (!found) {
            base_cstr_printf("Unknown command: %s. Type 'help' for a list of commands.\n", cmd_name);
        }
    }
    
    return 0;
}
