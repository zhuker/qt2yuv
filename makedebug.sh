#!/bin/bash

set -xue
FFMPEG="./ffmpeg/include"
FFMPEG_LIBS="./ffmpeg/lib/libswscale.a ./ffmpeg/lib/libavutil.a"
CFLAGS="-arch i386 -g -std=c99 -I$FFMPEG"

for each in *c ;do
gcc $CFLAGS -c $each
done
gcc -arch i386 -o qt2yuv *.o -framework QuickTime -framework Carbon $FFMPEG_LIBS
