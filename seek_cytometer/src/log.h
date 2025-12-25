// log.h
#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include "timers.h"
// 控制是否开启日志
#define LOG_ENABLE 1

// 是否开启彩色输出（仅用于支持 ANSI 的串口终端）
#define LOG_USE_COLOR 1

#if LOG_USE_COLOR
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"
#else
#define COLOR_RED     ""
#define COLOR_YELLOW  ""
#define COLOR_GREEN   ""
#define COLOR_RESET   ""
#endif

#if LOG_ENABLE
#define LOG_PRINT(level_color, level_str, fmt, ...) \
    do { \
        printf("%s[%s][%lu s] %s:%d %s(): " fmt "%s\r\n", \
               level_color, level_str, get_platform_time_s(), __FILE__, __LINE__, __func__, \
               ##__VA_ARGS__, COLOR_RESET); \
    } while (0)

#define LOG_INFO(fmt, ...)  LOG_PRINT(COLOR_GREEN,  "INFO",  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  LOG_PRINT(COLOR_YELLOW, "WARN",  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_PRINT(COLOR_RED,    "ERROR", fmt, ##__VA_ARGS__)

#else
// 禁用日志
#define LOG_INFO(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_ERROR(fmt, ...)
#endif

#endif // __LOG_H__
