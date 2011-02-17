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

#include <QuickTime/QuickTime.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct qtMovie {
    Movie myMovie;
    GWorldPtr myGWorld;
	
    Rect size;
    TimeValue movie_start_time;
    long next_frame_time;
	float msecScale;
	TimeScale timeScale;
	MediaHandler timecoder;
	TimeScale timecoderTimeScale;
	Track videoTrack;
	TimeScale videoMediaTimeScale;
	PixelAspectRatioImageDescriptionExtension pasp;
	FieldInfoImageDescriptionExtension2 fiel;
	
} qtMovie;

void qtMovie_update(qtMovie * capture);
qtMovie *qtMovie_open(char *filepath);
TimeValue qtMovie_setMovieTime(qtMovie *capture, int64_t timeValue);
void qtMovie_frameRateFromContainer(qtMovie *qtm, int *num, int *den);
void qtMovie_detectFrameRate(qtMovie *qtm, int *num, int *den);
