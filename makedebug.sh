#!/bin/bash

set -xue
CFLAGS="-g"
FFMPEG="./ffmpeg/include"
FFMPEG_LIBS="./ffmpeg/lib/libswscale.a ./ffmpeg/lib/libavutil.a"
gcc -arch i386 -std=c99 -o qt2yuv -I$FFMPEG $CFLAGS  qt2yuv.c -framework QuickTime -framework Carbon $FFMPEG_LIBS
