/*
 *  qt2yuv_util.c
 *  qt2yuv
 *
 *  Created by Alex Zhukov on 2/2/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "qt2yuv_util.h"

bool qt2yuv_debug = 0;
void yuv_debug(char *fmt, ...)
{
    if (qt2yuv_debug) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
}

void yuv_assertf(bool condition, char *func, char *format, int args)
{
    if (!condition) {
        yuv_debug(format, args);
        exit(1);
    }
}

void yuv_assert(bool condition, char *func, char *message)
{
    if (!condition) {
        yuv_debug("%s\n", message);
        exit(1);
    }
}

char *timeToString(int64_t msec, char *dst)
{
    long sec = msec / 1000;
    long ms = msec % 1000;
    long s = sec % 60;
    long m = sec % 3600 / 60;
    long h = sec / 3600;
    snprintf(dst, 13, "%02d:%02d:%02d.%03d", h, m, s, ms);
    return dst;
}
