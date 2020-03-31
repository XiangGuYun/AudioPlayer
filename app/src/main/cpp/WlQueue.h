//
// Created by 86139 on 2020/3/26.
//

#ifndef AUDIOPLAYER_WLQUEUE_H
#define AUDIOPLAYER_WLQUEUE_H

#include "pthread.h"
#include "queue"
#include "WlPlayStatus.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};

class WlQueue {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t lock;
    pthread_cond_t condition;
    WlPlayStatus *status = NULL;
public:
    WlQueue(WlPlayStatus *s);
    ~WlQueue();

    int push(AVPacket *packet);
    int pop(AVPacket *packet);
    int size();
    void clearAVPacket();
};


#endif //AUDIOPLAYER_WLQUEUE_H
