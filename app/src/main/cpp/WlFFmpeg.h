//
// Created by yangw on 2018-2-28.
//

#ifndef MYMUSIC_WLFFMPEG_H
#define MYMUSIC_WLFFMPEG_H

#include "WlCallJava.h"
#include "pthread.h"
#include "WlAudio.h"
#include "WlPlayStatus.h"

extern "C"
{
#include "libavformat/avformat.h"
#include <libavutil/time.h>
};


class WlFFmpeg {

public:
    WlCallJava *callJava = NULL;
    const char* url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx = NULL;
    WlAudio *audio = NULL;
    WlPlayStatus *playstatus = NULL;
    pthread_mutex_t init_mutex;
    bool exit = false;
    int duration = 0;
    pthread_mutex_t seek_mutex;
public:
    WlFFmpeg(WlPlayStatus *playstatus, WlCallJava *callJava, const char *url);

    ~WlFFmpeg();

    void prepared();

    void decodeFFmpegThread();

    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t secds);

    void setVolume(int percent);

    void setPitch(float pitch);

    void setTempo(float tempo);

};


#endif //MYMUSIC_WLFFMPEG_H
