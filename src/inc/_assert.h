#ifndef _INTERNAL_LIBC_ASSERT_H
#define _INTERNAL_LIBC_ASSERT_H

#define BLEND_COLOR_3B 0  // 支持8色
#define BLEND_COLOR_8B 1  // 支持256色
#define BLEND_COLOR_24B 2 // 支持24位色

#define CONSOLE_COLOR BLEND_COLOR_8B

#ifdef _cplusplus
extern "C"
{
#endif

#include "_stdio.h"
#define LIBC_PRINTF printf

#define CSI_START "\033["
#define CSI_END "\033[0m"

#if (CONSOLE_COLOR == BLEND_COLOR_3B)
/** 3bits 色定义 **/
/* output log front color */
#define F_BLACK "30m"   // 黑色
#define F_RED "31m"     // 红色
#define F_GREEN "32m"   // 绿色
#define F_YELLOW "33m"  // 黄色
#define F_BLUE "34m"    // 蓝色
#define F_MAGENTA "35m" // 品红
#define F_CYAN "36m"    // 青色
#define F_WHITE "37m"   // 白色

#define F_B_BLOCK "90m"   // 亮黑色
#define F_B_READ "91m"    // 亮红
#define F_B_GREEN "92m"   // 亮绿色
#define F_B_YELLOW "93m"  // 亮黄色
#define F_B_BLUE "94m"    // 亮蓝色
#define F_B_MAGENTA "95m" // 洋红色
#define F_B_CYAN "96m"    // 亮青色
#define F_B_WHITE "97m"   // 亮白色
#elif (CONSOLE_COLOR == BLEND_COLOR_8B)
/** 256色 **/
#define F_FMT_COLOR(X) "0;1;38;5;" X "m"

#define F_BLACK F_FMT_COLOR("0")   // 黑色
#define F_RED F_FMT_COLOR("1")     // 红色
#define F_GREEN F_FMT_COLOR("2")   // 绿色
#define F_YELLOW F_FMT_COLOR("3")  // 黄色
#define F_BLUE F_FMT_COLOR("4")    // 蓝色
#define F_MAGENTA F_FMT_COLOR("5") // 品红
#define F_CYAN F_FMT_COLOR("6")    // 青色
#define F_WHITE F_FMT_COLOR("7")   // 白色

#define F_B_BLOCK F_FMT_COLOR("8")    // 亮黑色
#define F_B_READ F_FMT_COLOR("9")     // 亮红
#define F_B_GREEN F_FMT_COLOR("10")   // 亮绿色
#define F_B_YELLOW F_FMT_COLOR("11")  // 亮黄色
#define F_B_BLUE F_FMT_COLOR("12")    // 亮蓝色
#define F_B_MAGENTA F_FMT_COLOR("13") // 洋红色
#define F_B_CYAN F_FMT_COLOR("14")    // 亮青色
#define F_B_WHITE F_FMT_COLOR("15")   // 亮白色

#define F_C_BLUE F_FMT_COLOR("27")         // 经典蓝
#define F_C_RED F_FMT_COLOR("93")          // 经典红
#define F_C_RED1 F_FMT_COLOR("99")         // 经典浅红
#define F_C_RED2 F_FMT_COLOR("207")        // 粉红色
#define F_C_GREEN F_FMT_COLOR("82")        // 经典绿
#define F_C_VIOLET F_FMT_COLOR("171")      // 紫色
#define F_C_GREE_YELLOW F_FMT_COLOR("118") // 浅绿黄色
#define F_C_YELLOW F_FMT_COLOR("226")      // 经典黄

#define F_C_WHITE F_FMT_COLOR("255") // 经典白
#else
/*** 24位色定义 ***/
#define F_FMT_COLOR(r, g, b) "38;2;" r ";" g ";" b "m"
#define F_COLOR F_FMT_COLOR("255", "0", "255")
#define F_LC F_FMT_COLOR("255", "0", "0")
#endif

#if 1
#define START_OUTPUT(color) LIBC_PRINTF("" CSI_START "" color "")
#define ASSERT_OUTPUR(fmt, ...) LIBC_PRINTF(fmt, ##__VA_ARGS__)
#define END_OUPUT LIBC_PRINTF(CSI_END)
#define END_BACKGROUP LIBC_PRINTF(B_DEF_COLOR)

#define INFO_OUTPUT(...)        \
    START_OUTPUT(F_B_WHITE);    \
    ASSERT_OUTPUR(__VA_ARGS__); \
    END_OUPUT

#define ERR_OUTPUT(...)         \
    START_OUTPUT(F_B_READ);     \
    ASSERT_OUTPUR(__VA_ARGS__); \
    END_OUPUT

#define WARN_OUTPUT(...)        \
    START_OUTPUT(F_C_YELLOW);   \
    ASSERT_OUTPUR(__VA_ARGS__); \
    END_OUPUT

#define MSG_OUTPUT(...)         \
    START_OUTPUT(F_C_GREEN);    \
    ASSERT_OUTPUR("[M/LOG] ");  \
    ASSERT_OUTPUR(__VA_ARGS__); \
    END_OUPUT

#ifdef DEBUG_TAG
#define DBG_OUTPUT(...)                          \
    SPEC_OUTPUT(F_B_CYAN, "[D/" DEBUG_TAG "] "); \
    START_OUTPUT(F_B_BLUE);                      \
    ASSERT_OUTPUR(__VA_ARGS__);                  \
    END_OUPUT
#else
#define DBG_OUTPUT(...)
#endif

#define SPEC_OUTPUT(color, ...) \
    START_OUTPUT(color);        \
    ASSERT_OUTPUR(__VA_ARGS__); \
    END_OUPUT

#define DEBUG_OUTPUT(color, ...)
    // START_OUTPUT(F_FMT_COLOR(#color))
    // MSG_OUTPUT(##__VA_ARGS__);
    // END_OUPUT

#define INFO_RESULT_OUTPUT(str) \
    SPEC_OUTPUT(F_C_GREEN, str)

#define LOG_ASSERT(x)                                 \
    if (!(x))                                         \
    {                                                 \
        SPEC_OUTPUT(F_C_RED2, "[L/LOG] Parameter ");  \
        SPEC_OUTPUT(F_C_BLUE, #x);                    \
        SPEC_OUTPUT(F_C_RED2, " Fail\r\n");           \
    }                                                 \
    else                                              \
    {                                                 \
        SPEC_OUTPUT(F_C_GREEN, "[L/LOG] Parameter "); \
        SPEC_OUTPUT(F_C_BLUE, #x);                    \
        SPEC_OUTPUT(F_C_GREEN, " OK\r\n");            \
    }

#define ERR_ASSERT(x)                     \
    if (!(x))                             \
    {                                     \
        ERR_OUTPUT("[E/LOG] Parameter "); \
        SPEC_OUTPUT(F_C_BLUE, #x);        \
        ERR_OUTPUT(" Check Fail\r\n");    \
    }

#define WARN_ASSERT(x)                     \
    if (!(x))                              \
    {                                      \
        WARN_OUTPUT("[W/LOG] Parameter "); \
        SPEC_OUTPUT(F_C_BLUE, #x);         \
        WARN_OUTPUT(" Check Fail\r\n");    \
    }
#endif

    void result_output(int result);
    void msg_output(const char *str, int value);
    void assertCallback(unsigned char flag, void *pfunc(void *p_arg));
    void __assert(const char *assertion, const char *filename,
                  unsigned int linenumber, register const char *function);

#define assert(x) \
    __assert(#x, __FILE__, __LINE__, __FUNCTION__)

#ifdef _cplusplus
}
#endif

#endif
