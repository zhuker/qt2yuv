/*                                                                                                                                                                                              
 * qt2yuv main                                                                                                                                                                                  
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
#include <libswscale/swscale.h>
#include <stdlib.h>
#include "qtmovie.h"
#include "qt2yuv_util.h"
#include "qt2yuv_img.h"

int W = 0;
int H = 0;


void writeFrame(int64_t timeValue, int size, void *data)
{
    int written = fwrite(data, size, 1, stdout);
    yuv_assert(1 == written, "writeFrame", "short write");
}

struct scale {
    struct SwsContext *sws;
	qt2yuv_pict src;
	qt2yuv_pict dst;
};

typedef struct scale scale_t;

/** Initializes YUVS to YUV420P converter. */
scale_t *scale_init(scale_t * scale, int W, int H, int dstW, int dstH, enum PixelFormat dstPixFmt)
{
    scale->sws = sws_getContext(W, H, PIX_FMT_YUYV422, dstW, dstH, dstPixFmt, SWS_BILINEAR | SWS_PRINT_INFO, NULL, NULL, NULL);
	yuv_assert(scale->sws != NULL, "open", "cant create sws context");

	scale->src.datasize = 0;
	scale->src.data[0] = NULL;
	scale->src.data[1] = NULL;
	scale->src.data[2] = NULL;
	scale->src.data[3] = NULL;
	scale->src.linesize[0] = 0;
	scale->src.linesize[1] = 0;
	scale->src.linesize[2] = 0;
	scale->src.linesize[3] = 0;
	scale->src.w = W;
	scale->src.h = H;
	
	qt2yuv_picture_alloc(&(scale->dst), dstPixFmt, dstW, dstH);
	return scale;
}

void scale_set_srcstride(scale_t * scale, int s0, int s1, int s2)
{
    scale->src.linesize[0] = s0;
    scale->src.linesize[1] = s1;
    scale->src.linesize[2] = s2;
    scale->src.linesize[3] = 0;
}

void scale_doScale(scale_t * scale, void *src_data)
{
	scale->src.data[0] = src_data;
    sws_scale(scale->sws, scale->src.data, scale->src.linesize, 0, scale->src.h, scale->dst.data,
              scale->dst.linesize);
}


typedef struct opts {
    char *filename;
	int scaleToWidth;
	int nth;
	int isInteractive;
	enum PixelFormat dstPixFmt;
	int detectFrameRate;
} opts_t;

static char* pixFmtNames[PIX_FMT_NB];
static char* pixFmtYSCSS[PIX_FMT_NB];

opts_t *parse_opts(opts_t *opts, int argc, char **argv) {
	int i = 1;
	for(i=1;i<argc;i++) {
		if (!strcmp(argv[i], "-d")) {
			qt2yuv_debug = 1;
		} else if (!strcmp(argv[i], "-f")) {
			opts->detectFrameRate = true;
		} else if (!strcmp(argv[i], "-i")) {
			opts->isInteractive = 1;
		} else if (!strcmp(argv[i], "-s")) {
			opts->scaleToWidth = atoi(argv[i+1]);
			opts->scaleToWidth = align16(opts->scaleToWidth);
			i++;
		} else if (!strcmp(argv[i], "-c")) {
			if (!strcmp("422", argv[i+1])) {
				opts->dstPixFmt = PIX_FMT_YUV422P;
			} else if (!strcmp("411", argv[i+1])) {
				opts->dstPixFmt = PIX_FMT_YUV411P;
			}
			i++;
		} else if (argv[i][0] != '-') {
			break;
		}
	}
	opts->filename = argv[i++];
	if (argc - 1 >= i) {
		opts->nth = atoi(argv[i]);
	}
	return opts;
}

void decodeFrame(qtMovie * capture, scale_t *scale, TimeValue myCurrTime) {

	TimeValue videoTv = TrackTimeToMediaTime(myCurrTime, capture->videoTrack);
	TimeValue videoTrackOffset =  GetTrackOffset(capture->videoTrack);
	yuv_debug("decodeFrame: videotv: %d off: %d\n", videoTv, videoTrackOffset);
	
	if (capture->timecoder) {
		TimeRecord tr;
		// convert edit duration to destination time code scale
		tr.base = nil;
		tr.scale = capture->timeScale;
		tr.value.lo = myCurrTime;
		tr.value.hi = 0;
		ConvertTimeScale(&tr, capture->timecoderTimeScale);

		TimeCodeDef tcdef = {0};
		TimeCodeRecord tcdata = {0};
		TCGetTimeCodeAtTime(capture->timecoder, tr.value.lo, NULL, &tcdef, &tcdata, NULL);
		printf("FRAME XTV=%d XVMTV=%d XNDFTC=%02d:%02d:%02d:%02d\n", myCurrTime, videoTv, (int32_t) tcdata.t.hours, (int32_t) tcdata.t.minutes, (int32_t) tcdata.t.seconds, (int32_t) tcdata.t.frames);
		yuv_debug("decodeFrame: %d %02d:%02d:%02d:%02d\n", myCurrTime, tcdata.t.hours, tcdata.t.minutes,tcdata.t.seconds,tcdata.t.frames);
	} else {
		printf("FRAME XTV=%d XVMTV=%d\n", myCurrTime, videoTv);
	}
	
	qtMovie_update(capture);
	PixMapHandle myPixMapHandle = GetGWorldPixMap(capture->myGWorld);
	LockPixels(myPixMapHandle);
	scale_set_srcstride(scale, GetPixRowBytes(myPixMapHandle), 0, 0);
	scale_doScale(scale, GetPixBaseAddr(myPixMapHandle));
	writeFrame(myCurrTime, scale->dst.datasize, scale->dst.data[0]);
	UnlockPixels(myPixMapHandle);
}

int main(int argc, char **argv)
{
	pixFmtNames[PIX_FMT_YUV420P] = "420mpeg2";
	pixFmtNames[PIX_FMT_YUV422P] = "422";
	pixFmtNames[PIX_FMT_YUV411P] = "411";
	pixFmtYSCSS[PIX_FMT_YUV420P] = "420MPEG2";
	pixFmtYSCSS[PIX_FMT_YUV422P] = "422";
	pixFmtYSCSS[PIX_FMT_YUV411P] = "411";
	
    if (argc < 2) {
        printf("qt2yuv v0.4.7\n");
        printf("usage: qt2yuv -i -d -s [width] -c [colorspace] pathtofile.mov [nthFrame]\n");
        printf("\t-i interactive seek mode\n");
        printf("\t-d debug\n");
        printf("\t-s scale to width (always rounded by 16)\n");
        printf("\t-c colorspace - output colorspace (default: 420mpeg2) also available: 422 411\n");
        printf("\t-f - detect frame rate\n");
        printf("\t[nthFrame] render only nth frame (default: 1)\n");
        exit(1);
    }
	opts_t opts = {NULL, 0, 1, 0, PIX_FMT_YUV420P, 0};
	parse_opts(&opts, argc, argv);
	yuv_debug("output colorspace %s\n", pixFmtNames[opts.dstPixFmt]);
	
	qtMovie *qtm = qtMovie_open(opts.filename);
	
    scale_t scale;
    W = qtm->size.right - qtm->size.left;
    W += (W & 1);
    H = qtm->size.bottom - qtm->size.top;
	int dstW = W;
	int dstH = H;
	if (opts.scaleToWidth) {
		dstW = opts.scaleToWidth;
		dstH = (((double)opts.scaleToWidth / W) * H);
		dstH = align16(dstH);
	}
    scale_init(&scale, W, H, dstW, dstH, opts.dstPixFmt);
	
    int num = 0;
    int den = 0;
	if (opts.detectFrameRate) {
		qtMovie_detectFrameRate(qtm, &num, &den);
	} else {
		qtMovie_frameRateFromContainer(qtm, &num, &den);
	}
	
    yuv_debug("num: %d den: %d %.2ffps\n", num, den,
              (double) num / (double) den);
	
	char interlaced = 'p';
	if (qtm->fiel.fields == 2 && (qtm->fiel.detail % 2) == 0) {
		interlaced = 'b';
	} else if (qtm->fiel.fields == 2 && (qtm->fiel.detail % 2) == 1) {
		interlaced = 't';
	}
	
    printf("YUV4MPEG2 W%d H%d F%d:%d I%c A%d:%d C%s XYSCSS=%s XTS=%d XVMTS=%d\n", 
		   dstW, dstH, 
		   num, den, 
		   interlaced, 
		   qtm->pasp.hSpacing, qtm->pasp.vSpacing, 
		   pixFmtNames[opts.dstPixFmt], pixFmtYSCSS[opts.dstPixFmt],
           qtm->timeScale, qtm->videoMediaTimeScale);
	
	if (opts.isInteractive) {
		while(!feof(stdin)) {
			int64_t timeValue = 0;
			if (1 == fscanf(stdin, "%lld", &timeValue)) {
				TimeValue myCurrTime = qtMovie_setMovieTime(qtm, timeValue);
				decodeFrame(qtm, &scale, myCurrTime);
			}
		}
	} else {
		int64_t start = timeMillis();
		int i = 0;
		for (i = 0;; i++) {
			TimeValue myCurrTime = qtMovie_setMovieTime(qtm, qtm->next_frame_time);
			
			if (i % opts.nth == 0) {
				yuv_debug("frame: %d\n", i);
				decodeFrame(qtm, &scale, myCurrTime);
			}
			
			// increment counters
			OSType myType = VisualMediaCharacteristic;
			short flags = nextTimeMediaSample;
			GetMovieNextInterestingTime(qtm->myMovie, flags, 1,
										&myType, myCurrTime, 1,
										&qtm->next_frame_time, NULL);
			yuv_assert(GetMoviesError() == noErr, "grab",
					   "Couldn't GetMovieNextInterestingTime()");
			if (qtm->next_frame_time == -1) {
				break;
			}
		}
		
		int64_t time = timeMillis() - start;
		yuv_debug("%lld fps. %d frames in %lld msec\n", i * 1000LL / time, i, time);
		
		//TODO: close everything
	}
}
