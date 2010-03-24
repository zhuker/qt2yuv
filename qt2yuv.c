#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <libswscale/swscale.h>
#include <stdlib.h>
#include "qtmovie.h"
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
int W = 0;
int H = 0;


void writeFrame(int64_t timeValue, int size, void *data)
{
    int written = fwrite(data, size, 1, stdout);
    yuv_assert(1 == written, "writeFrame", "short write");
}

int64_t timeMillis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

struct scale {
    struct SwsContext *sws;
    int src_stride[3];
    int dst_stride[3];
    uint8_t *src[3];
    uint8_t *dst[3];
	
    void *src_data;
    void *dst_data;
    int W;
    int H;
    int dstW;
    int dstH;
    int dst_size;
};

typedef struct scale scale_t;

/** Initializes YUVS to YUV420P converter. */
scale_t *scale_init(scale_t * scale, int W, int H, int dstW, int dstH)
{
    scale->sws =
	sws_getContext(W, H, PIX_FMT_YUYV422, dstW, dstH, PIX_FMT_YUV420P,
				   SWS_BILINEAR, NULL, NULL, NULL);
    yuv_assert(scale->sws != NULL, "open", "cant create sws context");
    scale->src_stride[0] = 2 * W;
    scale->src_stride[1] = 0;
    scale->src_stride[2] = 0;
    scale->W = W;
    scale->H = H;
    int uStride = (dstW >> 1);
    int vStride = (dstW >> 1);
    scale->dst_stride[0] = dstW;
    scale->dst_stride[1] = uStride;
    scale->dst_stride[2] = vStride;
	
    int dst_size = dstW * (dstH + (dstH >> 1));
	int ySize = dstW * dstH;
	int uSize = ySize >> 2;
    scale->dstW = dstW;
    scale->dstH = dstH;
    scale->dst_size = dst_size;
    scale->dst_data = malloc(dst_size);
    scale->dst[0] = scale->dst_data;
    scale->dst[1] = scale->dst_data + ySize;
    scale->dst[2] = scale->dst_data + ySize + uSize;
	return scale;
}

void scale_set_srcstride(scale_t * scale, int s0, int s1, int s2)
{
    scale->src_stride[0] = s0;
    scale->src_stride[1] = s1;
    scale->src_stride[2] = s2;
}

void scale_doScale(scale_t * scale, void *src_data)
{
    uint8_t *src[3] = { src_data, NULL, NULL };
    sws_scale(scale->sws, src, scale->src_stride, 0, scale->H, scale->dst,
              scale->dst_stride);
}


typedef struct opts {
    char *filename;
	int scaleToWidth;
	int nth;
	int isInteractive;
} opts_t;

opts_t *parse_opts(opts_t *opts, int argc, char **argv) {
	int i = 1;
	for(i=1;i<argc;i++) {
		if (!strcmp(argv[i], "-d")) {
			qt2yuv_debug = 1;
		} else if (!strcmp(argv[i], "-i")) {
			opts->isInteractive = 1;
		} else if (!strcmp(argv[i], "-s")) {
			opts->scaleToWidth = atoi(argv[i+1]);
			opts->scaleToWidth = align16(opts->scaleToWidth);
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
	yuv_debug("videotv: %d off: %d\n", videoTv, videoTrackOffset);
	
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
		yuv_debug("%d %02d:%02d:%02d:%02d\n", myCurrTime, tcdata.t.hours, tcdata.t.minutes,tcdata.t.seconds,tcdata.t.frames);
	} else {
		printf("FRAME XTV=%d XVMTV=%d\n", myCurrTime, videoTv);
	}
	
	qtMovie_update(capture);
	PixMapHandle myPixMapHandle = GetGWorldPixMap(capture->myGWorld);
	LockPixels(myPixMapHandle);
	scale_set_srcstride(scale, GetPixRowBytes(myPixMapHandle), 0, 0);
	scale_doScale(scale, GetPixBaseAddr(myPixMapHandle));
	writeFrame(myCurrTime, scale->dst_size, scale->dst_data);
	UnlockPixels(myPixMapHandle);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("qt2yuv v0.4.3\n");
        printf("usage: qt2yuv -i -d -s [width] pathtofile.mov [nthFrame]\n");
        printf("\t-i interactive seek mode\n");
        printf("\t-d debug\n");
        printf("\t-s scale to width (always rounded by 16)\n");
        printf("\t[nthFrame] render only nth frame (default: 1)\n");
        exit(1);
    }
	opts_t opts = {NULL, 0, 1, 0};
	parse_opts(&opts, argc, argv);
	
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
    scale_init(&scale, W, H, dstW, dstH);
	
    int num = 0;
    int den = 0;
    qtMovie_detectFrameRate(qtm, &num, &den);
	
    yuv_debug("num: %d den: %d %.2ffps\n", num, den,
              (double) num / (double) den);
	
    printf("YUV4MPEG2 W%d H%d F%d:%d Ip A1:1 C420mpeg2 XTS=%d XVMTS=%d\n", dstW, dstH, num, den,
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
				decodeFrame(qtm, &scale, myCurrTime);
			}
			
			// increment counters
			OSType myType = VisualMediaCharacteristic;
			GetMovieNextInterestingTime(qtm->myMovie, nextTimeStep, 1,
										&myType, myCurrTime, 1,
										&qtm->next_frame_time, NULL);
			yuv_assert(GetMoviesError() == noErr, "grab",
					   "Couldn't GetMovieNextInterestingTime()");
			if (qtm->next_frame_time == -1) {
				decodeFrame(qtm, &scale, myCurrTime);
				break;
			}
		}
		
		int64_t time = timeMillis() - start;
		yuv_debug("%lld fps. %d frames in %lld msec\n", i * 1000LL / time, i, time);
		
		//TODO: close everything
	}
}
