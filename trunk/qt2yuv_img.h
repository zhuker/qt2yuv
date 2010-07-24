/*
 *  qt2yuv_img.h
 *  qt2yuv
 *
 *  Created by Alex Zhukov on 7/24/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include <sys/types.h>
#include <libavutil/pixfmt.h>

#include <libswscale/swscale.h>

typedef struct qt2yuv_pict {
	uint8_t *data[4];
	int linesize[4];
	int datasize;
	int w;
	int h;
} qt2yuv_pict;

int qt2yuv_picture_alloc(qt2yuv_pict *pict, enum PixelFormat fmt, int w, int h);

void writepnm(int w, int h, void *data, char *filename);
