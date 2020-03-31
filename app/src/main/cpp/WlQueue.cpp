//
// Created by 86139 on 2020/3/26.
//

#include "WlQueue.h"
#include "LogUtils.h"

WlQueue::WlQueue(WlPlayStatus *s) {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);
    status = s;
}

WlQueue::~WlQueue() {
    clearAVPacket();
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&condition);
}

int WlQueue::push(AVPacket *packet) {
    pthread_mutex_lock(&lock);
    queuePacket.push(packet);
    LOGD("放入一个AVPacket到队列中，个数为 %d", queuePacket.size());
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&lock);
    return 0;
}

int WlQueue::pop(AVPacket *packet) {
    pthread_mutex_lock(&lock);
    while (status != NULL && !status->exit){
        if(queuePacket.size()>0){
            AVPacket *p = queuePacket.front();
            /*
             * 设置对给定数据包所描述的数据的新引用
             * 返回0表示成功
             */
            if(av_packet_ref(packet, p) == 0){
                queuePacket.pop();
            }
            //拷贝引用之后需要释放引用
            av_packet_free(&p);
            av_free(p);
            p = NULL;
            LOGD("从队列里面取出一个AVPacket，还剩下 %d", queuePacket.size());
            break;
        } else{
            pthread_cond_wait(&condition, &lock);
        }
    }
    pthread_mutex_unlock(&lock);
    return 0;
}

int WlQueue::size() {
    int size = 0;
    pthread_mutex_lock(&lock);
    size = queuePacket.size();
    pthread_mutex_unlock(&lock);
    return size;
}

void WlQueue::clearAVPacket() {
    pthread_cond_signal(&condition);
    pthread_mutex_lock(&lock);
    while (!queuePacket.empty()){
        AVPacket *packet = queuePacket.front();
        queuePacket.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&lock);
}
