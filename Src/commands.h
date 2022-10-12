#ifndef COMMANDS_H_
#define COMMANDS_H_
#include <stdbool.h>
#include <stddef.h>

struct command_description {
    const char *name;
    // args is string with all arguments. It returns 1 if runs successfully,
    // 0 otherwise
    bool (*function)(char *args);
    const char *help;
};

#define for_each_command(command_desc_ptr, command_index)                      \
    for (command_index = 0, command_desc_ptr = &command_table[0];              \
         command_index < command_table_size;                                   \
         command_desc_ptr = &command_table[++command_index])

extern struct command_description command_table[];
extern size_t command_table_size;

void execute_command_line(char *line);

#endif /* COMMANDS_H_ */
