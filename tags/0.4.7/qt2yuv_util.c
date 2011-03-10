/*                                                                                                                                                                                              
 * qt2yuv                                                                                                                                                                                  
 * Copyright (c) 2009-2010 Alex Zhukov                                                                                                                                                      
 *                                                                                                                                                                                              
 * This file is part of qt2yuv.                                                                                                                                                                 
 *                                                                                                                                                                                              
 * qt2yuv is free software; you can redistribute it and/or                                                                                                                                      
 * modify it under the terms of the GNU Lesser General Public                                                                                                                                   
 * License as published by the Free Software Foundation; either                                                                                                                                 
 * version 2.1 of the License, or (at your option) any later version.                                                                                                                           
 *                                                                                                                                                                                              
 * FFmpeg is distributed in the hope that it will be useful,                                                                                                                                    
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                                                                                                                               
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                                                                                                                            
 * Lesser General Public License for more details.                                                                                                                                              
 *                                                                                                                                                                                              
 * You should have received a copy of the GNU Lesser General Public                                                                                                                             
 * License along with FFmpeg; if not, write to the Free Software                                                                                                                                
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA                                                                                                                 
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


int64_t timeMillis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}