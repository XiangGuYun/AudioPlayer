//
// Created by yangw on 2018-2-28.
//

#include "WlCallJava.h"
#include <android/log.h>
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ywl5320",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG,"ywl5320",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ywl5320",FORMAT,##__VA_ARGS__);

WlCallJava::WlCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {

    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass  jlz = jniEnv->GetObjectClass(jobj);
    if(!jlz)
    {
        return;
    }
    jmidPrepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
    jmidLoaded = env->GetMethodID(jlz, "onCallLoaded", "(Z)V");
    jmidProgress = env->GetMethodID(jlz, "progress", "(II)V");
    jmidError = env->GetMethodID(jlz, "error", "(ILjava/lang/String;)V");
    jmidComplete = env->GetMethodID(jlz, "onComplete", "()V");
}

void WlCallJava::onCallPrepared(int type) {
    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmidPrepared);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            LOGE("发生了错误")
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmidPrepared);
        javaVM->DetachCurrentThread();
    }

}

void WlCallJava::test() {
    LOGD("调用了WlCallJava的test方法")
}

void WlCallJava::onCallLoaded(int type, bool load) {
    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmidLoaded, load);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            LOGE("发生了错误")
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmidLoaded, load);
        javaVM->DetachCurrentThread();
    }
}

void WlCallJava::onCallProgress(int type, int currentTime, int totalTime) {
    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmidProgress, currentTime, totalTime);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            LOGE("发生了错误")
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmidProgress, currentTime, totalTime);
        javaVM->DetachCurrentThread();
    }

}

WlCallJava::~WlCallJava() {

}

void WlCallJava::onCallError(int type, int code, char *msg) {
    if(type == MAIN_THREAD)
    {
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmidError, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        LOGE("发生了错误1")
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            return;
        }
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmidError, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }
}

void WlCallJava::onCallComplete(int type) {
    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmidComplete);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmidComplete);
        javaVM->DetachCurrentThread();
    }
}
