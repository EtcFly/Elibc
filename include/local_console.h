#ifndef _LOCAL_CONSOLE_H
#define _LOCAL_CONSOLE_H

typedef int (*lc_console_callback_t)(const char *s);

#define MAX_LC_CONSOLE_SIZE 4
#define MAX_LC_CONSOLE_NAME_SIZE 16

int set_active_console(const char *console_name);
lc_console_callback_t get_local_console(const char *console_name);
int set_local_console(const char *console_name, lc_console_callback_t console_cb);
const char *get_active_console_name(void);
lc_console_callback_t get_active_console(void);

#endif
