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

#include "qtmovie.h"
#include "qt2yuv_util.h"

void _media_getPixelAspect(Media m, PixelAspectRatioImageDescriptionExtension *pasp) {
	SampleDescriptionHandle descH = (SampleDescriptionHandle)NewHandle(sizeof(Handle));
	Handle handle = NewHandle(sizeof(Handle)); 
	GetMediaSampleDescription(m, 1, descH);
	long extensionCount = 0;
	CountImageDescriptionExtensionType((ImageDescriptionHandle)descH, kICMImageDescriptionPropertyID_PixelAspectRatio, &extensionCount);
	if (extensionCount != 0) {
		GetImageDescriptionExtension((ImageDescriptionHandle)descH, &handle, kICMImageDescriptionPropertyID_PixelAspectRatio, 1);
		PixelAspectRatioImageDescriptionExtension *p = (PixelAspectRatioImageDescriptionExtension*) (*handle);
		pasp->hSpacing = CFSwapInt32BigToHost(p->hSpacing);
		pasp->vSpacing = CFSwapInt32BigToHost(p->vSpacing);
		yuv_debug("pasp: %d:%d\n", pasp->hSpacing, pasp->vSpacing);
	}
	DisposeHandle((Handle)descH);
	DisposeHandle(handle);
}

void _media_getFieldInfo(Media m, FieldInfoImageDescriptionExtension2 *f) {
	SampleDescriptionHandle descH = (SampleDescriptionHandle)NewHandle(sizeof(Handle));
	Handle handle = NewHandle(sizeof(Handle)); 
	
	GetMediaSampleDescription(m, 1, descH);
	long extensionCount = 0;
	CountImageDescriptionExtensionType((ImageDescriptionHandle)descH, kICMImageDescriptionPropertyID_FieldInfo, &extensionCount);
	if (extensionCount != 0) {
		GetImageDescriptionExtension((ImageDescriptionHandle)descH, &handle, kICMImageDescriptionPropertyID_FieldInfo, 1);
		FieldInfoImageDescriptionExtension2 *fiel = (FieldInfoImageDescriptionExtension2*) (*handle);
		yuv_debug("fiel: %d %d\n", fiel->fields, fiel->detail);
		f->fields = fiel->fields;
		f->detail = fiel->detail;
	}
	DisposeHandle((Handle)descH);
	DisposeHandle(handle);
}

qtMovie *qtMovie_open(char *filepath) {
    Handle myDataRef = nil;
    OSType myDataRefType = 0;
    OSErr myErr = noErr;
    qtMovie *capture = malloc(sizeof(qtMovie));
	memset(capture, 0, sizeof(qtMovie));
    short myResID = 0;
	capture->pasp.hSpacing = 1;
	capture->pasp.vSpacing = 1;
	capture->fiel.fields = 1;
	capture->fiel.detail = 1;
	
    EnterMovies();
    // no old errors please
    ClearMoviesStickyError();
    CFStringRef inPath =
	CFStringCreateWithCString(kCFAllocatorDefault, filepath,
							  kCFStringEncodingUTF8);
    yuv_assert((inPath != nil), "open",
               "couldnt create CFString from a string");
	
    // create the data reference
    myErr =
	QTNewDataReferenceFromFullPathCFString(inPath, kQTPOSIXPathStyle,
										   0, &myDataRef, &myDataRefType);
    yuv_assert(myErr == noErr, "open",
               "Couldn't create QTNewDataReferenceFromFullPathCFString().");
    myErr =
	NewMovieFromDataRef(&capture->myMovie,
						newMovieActive | newMovieAsyncOK,
						&myResID, myDataRef, myDataRefType);
	
	// dispose of the data reference handle - we no longer need it
    DisposeHandle(myDataRef);
    yuv_assertf(myErr == noErr, "open",
                "Couldn't create a NewMovieFromDataRef() - error is %d.\n",
                myErr);
	
    GetMovieBox(capture->myMovie, &capture->size);
    myErr = QTNewGWorld(&capture->myGWorld, kYUVSPixelFormat,
                        &capture->size, nil, nil, 0);
    yuv_assert(myErr == noErr, "open",
               "couldnt create QTNewGWorld() for output image");
    SetMovieGWorld(capture->myMovie, capture->myGWorld, nil);
	
	
    capture->timeScale = GetMovieTimeScale(capture->myMovie);
    yuv_debug("timescale: %lld\n", capture->timeScale);
    capture->msecScale = (float) 1000 / capture->timeScale;
    yuv_debug("msecScale: %f\n", capture->msecScale);
	
	Track timeCodeTrack = GetMovieIndTrackType(capture->myMovie, 1, TimeCodeMediaType, movieTrackMediaType);
	if (timeCodeTrack) {
		Media theMedia = GetTrackMedia(timeCodeTrack);
		capture->timecoder = GetMediaHandler( theMedia );
		capture->timecoderTimeScale = GetMediaTimeScale(theMedia);
	}
	
	Track videoTrack = GetMovieIndTrackType(capture->myMovie, 1, VideoMediaType, movieTrackMediaType);
	if (videoTrack) {
		capture->videoTrack = videoTrack;
		Media m = GetTrackMedia(videoTrack);
		capture->videoMediaTimeScale = GetMediaTimeScale(m);
		_media_getPixelAspect(m, &capture->pasp);
		_media_getFieldInfo(m, &capture->fiel);

	}

	return capture;
}




void qtMovie_update(qtMovie * capture)
{
    // invalidates the movie's display state so that the Movie Toolbox
    // redraws the movie the next time we call MoviesTask
    UpdateMovie(capture->myMovie);
    yuv_assert(GetMoviesError() == noErr, "grab", "Couldn't UpdateMovie()");
	
    // service active movie (= redraw immediately)
    MoviesTask(capture->myMovie, 0L);
    yuv_assert(GetMoviesError() == noErr, "grab",
               "MoviesTask() didn't succeed");
}

TimeValue qtMovie_setMovieTime(qtMovie *capture, int64_t timeValue) {
	char timeStr[15] = {0};
	SetMovieTimeValue(capture->myMovie, timeValue);
	yuv_assert(GetMoviesError() == noErr, "grab", "Couldn't SetMovieTimeValue()");
	
	TimeValue myCurrTime = GetMovieTime(capture->myMovie, NULL);
	int64_t pts = (int64_t) (((float) myCurrTime) * capture->msecScale);
	timeToString(pts, timeStr);
	yuv_debug("qtMovie_setMovieTime: %lld %lld %s\n", (int64_t) myCurrTime, pts, timeStr);
	return myCurrTime;
}

void qtMovie_detectFrameRate(qtMovie *qtm, int *num, int *den)
{
    OSType myType = VisualMediaCharacteristic;
    char *timeStr = strdup("00:00:00.000");
    long next_frame_time = -1;
    int fn = -1;
    int64_t pts = -1;
    do {
        fn++;
        SetMovieTimeValue(qtm->myMovie, next_frame_time);
        yuv_assert(GetMoviesError() == noErr, "grab",
                   "Couldn't SetMovieTimeValue()");
        TimeValue myCurrTime = GetMovieTime(qtm->myMovie, NULL);
        pts = myCurrTime * 1000LL / qtm->timeScale;
        timeStr = timeToString(pts, timeStr);
        yuv_debug("%d %lld %lld %s\n", fn, (int64_t) myCurrTime, pts, timeStr);
        GetMovieNextInterestingTime(qtm->myMovie, nextTimeStep, 1,
                                    &myType, myCurrTime, 1, &next_frame_time,
                                    NULL);
        if (pts >= 10000LL) {
			break;
        }
    } while (next_frame_time != -1);
    *num = fn * 1000;
    *den = (int) pts;
}
