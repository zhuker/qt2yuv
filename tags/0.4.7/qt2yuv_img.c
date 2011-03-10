/*
 *  qt2yuv_img.c
 *  qt2yuv
 *
 *  Created by Alex Zhukov on 7/24/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include "qt2yuv_img.h"
#include "qt2yuv_util.h"

void writepnm(int w, int h, void *data, char *filename)
{
    FILE *fd = fopen(filename, "w");
    yuv_assert(fd != NULL, "writepnm", "cant open file for writting");
    fprintf(fd, "P5\n%d %d\n255\n", w, h);
    int written = fwrite(data, w * h, 1, fd);
    yuv_assert(1 == written, "writepnm", "short write");
    fclose(fd);
}

int qt2yuv_picture_alloc(qt2yuv_pict *pict, enum PixelFormat fmt, int w, int h) {
	int datasize = 0;
	int lumaSize = w*h;
	int chromaSize = 0;
	pict->linesize[0] = w;
	pict->linesize[3] = 0;
	switch (fmt) {
		case PIX_FMT_YUV420P:
			datasize = w*h*3/2;
			chromaSize = (w>>1) * (h>>1);
			pict->linesize[1] = w >> 1;
			pict->linesize[2] = w >> 1;
			break;
		case PIX_FMT_YUV411P:
			datasize = w*h*3/2;
			chromaSize = (w>>1) * (h>>1);
			pict->linesize[1] = w >> 2;
			pict->linesize[2] = w >> 2;
			break;
		case PIX_FMT_YUV422P:
			datasize = w*h*2;
			chromaSize = (w>>1) * h;
			pict->linesize[1] = w >> 1;
			pict->linesize[2] = w >> 1;
			break;
		default:
			fprintf(stderr, "pixel format not supported\n");
			break;
	}
	unsigned char *data = malloc(datasize);
	pict->data[0] = data;
	pict->data[1] = data + lumaSize;
	pict->data[2] = data + lumaSize + chromaSize;
	pict->data[3] = NULL;
	pict->w = w;
	pict->h = h;
	pict->datasize = datasize;
	return 0;
}
