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
int64_t timeMillis();

#define align16(x) (16 + ((x - 1) & ~15))
