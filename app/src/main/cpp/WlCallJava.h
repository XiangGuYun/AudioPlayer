//
// Created by 86139 on 2020/3/25.
//

#ifndef MYMUSIC_WLCALLJAVA_H
#define MYMUSIC_WLCALLJAVA_H

#include "jni.h"
#include <linux/stddef.h>
#include <android/log.h>

#define MAIN_THREAD 0
#define CHILD_THREAD 1


class WlCallJava {

public:
    _JavaVM *javaVM = nullptr;
    JNIEnv *jniEnv = nullptr;
    jobject jobj;

    jmethodID jmidPrepared;
    jmethodID jmidLoaded;
    jmethodID jmidProgress;
    jmethodID jmidError;
    jmethodID jmidComplete;
public:
    WlCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj);
    ~WlCallJava();

    void onCallPrepared(int type);

    void onCallLoaded(int type, bool load);

    void onCallProgress(int type, int currentTime, int totalTime);

    void onCallError(int type, int code, char *msg);

    void onCallComplete(int type);

    void test();
};


#endif //MYMUSIC_WLCALLJAVA_H
