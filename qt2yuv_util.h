/*
 *  qt2yuv_util.h
 *  qt2yuv
 *
 *  Created by Alex Zhukov on 2/2/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

extern bool qt2yuv_debug;
void yuv_debug(char *fmt, ...);
void yuv_assertf(bool condition, char *func, char *format, int args);
void yuv_assert(bool condition, char *func, char *message);
char *timeToString(int64_t msec, char *dst);
#define align16(x) (16 + ((x - 1) & ~15))
