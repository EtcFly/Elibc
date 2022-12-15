#include "_assert.h"
#include "_stdio.h"

void result_output(int result)
{
    if (result == 0)
    {
        INFO_RESULT_OUTPUT("[OK]\r\n");
    }
    else if (result == -1)
    {
        INFO_RESULT_OUTPUT("[Fail]\r\n");
    }
}

void assertCallback(unsigned char flag, void *pfunc(void *p_arg))
{
    if (!flag)
    {
        INFO_OUTPUT("assert callback function %p\r\n", pfunc);
        pfunc((void *)0);
    }
}

void msg_output(const char *str, int value)
{
    if (str != NULL)
    {
        INFO_OUTPUT("%s ....... ", str);
    }

    if (value == 0 || value == -1)
    {
        result_output(value);
        return;
    }
}

void __assert(const char *assertion, const char *filename,
              unsigned int linenumber, register const char *function)
{
    (void)assertion;
    (void)filename;
    (void)linenumber;
    (void)function;
    printf("%s:%d:%s %s\r\n", filename, linenumber, assertion, function);
}