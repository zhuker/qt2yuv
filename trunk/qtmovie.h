/*
 *  qtmovie.h
 *  qt2yuv
 *
 *  Created by Alex Zhukov on 2/2/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
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
	bool preved;
	float msecScale;
	TimeScale timeScale;
	MediaHandler timecoder;
	TimeScale timecoderTimeScale;
	Track videoTrack;
	TimeScale videoMediaTimeScale;
} qtMovie;

void qtMovie_update(qtMovie * capture);
qtMovie *qtMovie_open(char *filepath);
TimeValue qtMovie_setMovieTime(qtMovie *capture, int64_t timeValue);
void qtMovie_detectFrameRate(qtMovie *qtm, int *num, int *den);
