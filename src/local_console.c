#include "local_console.h"
#include "types.h"
#include "string.h"

static char __lc_console_name[MAX_LC_CONSOLE_SIZE][MAX_LC_CONSOLE_NAME_SIZE];
static lc_console_callback_t __lc_console[MAX_LC_CONSOLE_SIZE];
static __libc_int8_t __lc_console_size = 0;
static lc_console_callback_t __lc_active_console;
static char __lc_active_console_name[MAX_LC_CONSOLE_NAME_SIZE];

int set_local_console(const char *console_name, lc_console_callback_t console_cb)
{
    if (NULL == console_cb || __lc_console_size >= MAX_LC_CONSOLE_SIZE)
    {
        return -1;
    }

    for (int i = 0; i < __lc_console_size; i++) // 去重
    {
        if (strcmp(console_name, __lc_console_name[i]) == 0)
        {
            __lc_console[i] = console_cb;
            return 0;
        }
    }

    int len = strlen(console_name);
    if (NULL != console_name)
    {
        memset(__lc_console_name[__lc_console_size], 0, MAX_LC_CONSOLE_NAME_SIZE);
        strncpy(__lc_console_name[__lc_console_size], console_name, len >= MAX_LC_CONSOLE_NAME_SIZE ? (len - 1) : len);
    }
    __lc_console[__lc_console_size] = console_cb;

    if (NULL == __lc_active_console)
    {
        __lc_active_console = console_cb;
        strcpy(__lc_active_console_name, __lc_console_name[__lc_console_size]);
    }

    return 0;
}

lc_console_callback_t get_local_console(const char *console_name)
{
    if (NULL == console_name)
    {
        return NULL;
    }

    for (int i = 0; i < __lc_console_size; i++)
    {
        if (strcmp(console_name, __lc_console_name[i]) == 0)
        {
            return __lc_console[i];
        }
    }
    return NULL;
}

int set_active_console(const char *console_name)
{
    lc_console_callback_t active_console;
    active_console = get_local_console(console_name);
    if (NULL == active_console)
    {
        return -1;
    }

    __lc_active_console = active_console;
    memset(__lc_active_console_name, 0, sizeof(__lc_active_console_name));
    strncpy(__lc_active_console_name, console_name, MAX_LC_CONSOLE_NAME_SIZE);
    return 0;
}

lc_console_callback_t get_active_console(void)
{
    return __lc_active_console;
}

const char *get_active_console_name(void)
{
    return __lc_active_console_name;
}
