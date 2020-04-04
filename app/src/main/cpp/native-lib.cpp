#include <jni.h>
#include <string>
#include <android/log.h>
#include <unistd.h>
#include "pthread.h"
#include "queue"
#include "WlCallJava.h"
#include "WlFFmpeg.h"
#include "LogUtils.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C"
{
#include "include/libavcodec/avcodec.h"
#include <libavformat/avformat.h>
}

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ywl5320",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG,"ywl5320",FORMAT,##__VA_ARGS__);


/**
 * 创建C++项目并导入FFMPEG动态库
 */
extern "C"
JNIEXPORT jstring JNICALL
Java_com_ywl5320_myplayer_Demo_stringFromJNI(JNIEnv *env, jobject instance) {
    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                LOGI("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGI("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGI("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
    std::string hello = "hello from jni";
    return env->NewStringUTF(hello.c_str());
}

/**
 * C++多线程、生产者和消费者模型以及C++全局调用Java方法
 */
pthread_t thread;

void *normalCallback(void *data) {
    LOGD("create normal thread from C++")
    pthread_exit(&thread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_ThreadDemo_normalThread(JNIEnv *env, jobject thiz) {
    pthread_create(&thread, NULL, normalCallback, NULL);
}

pthread_t producer;
pthread_t consumer;
pthread_mutex_t mutex;
pthread_cond_t condition;
std::queue<int> queue;

bool isExit = false;

void *producerCallback(void *data) {
    while (!isExit) {
        pthread_mutex_lock(&mutex);//加锁，类似于Java中的lock.lock()
        queue.push(1);
        LOGD("生产者生产了一个产品，通知消费者消费，产品数量为 %d", queue.size());
        pthread_cond_signal(&condition);//唤醒消费者线程，类似于Java中的condition.signal()
        pthread_mutex_unlock(&mutex);//解锁，类似于Java中的lock.unlock()
        sleep(5);
    }
    pthread_exit(&producer);
}

void *consumerCallback(void *data) {
    while (!isExit) {
        pthread_mutex_lock(&mutex);//加锁，类似于Java中的lock.lock()
        if (queue.size() > 0) {
            queue.pop();
            LOGD("消费者消费了一个产品，产品数量为 %d", queue.size());
        } else {
            LOGD("没有产品可以消费了，等待中")
            pthread_cond_wait(&condition, &mutex);//线程处于无限期等待状态，类似于Java中的condition.wait()
        }
        pthread_mutex_unlock(&mutex);//解锁，类似于Java中的lock.unlock()
        usleep(1000 * 500);
    }
    pthread_exit(&consumer);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_ThreadDemo_mutexThread(JNIEnv *env, jobject thiz) {
    for (int i = 0; i < 10; ++i) {
        queue.push(i);
    }
    /*
     * 类似于如下Java代码，mutex代表互斥锁，跟lock意思相近
     * Lock lock = new ReentrantLock()
     */
    pthread_mutex_init(&mutex, NULL);
    /*
     * 类似于如下Java代码
     * Condition condition = new Condition()
     */
    pthread_cond_init(&condition, NULL);
    /*
     * 类似于如下Java代码
     * Runnable producerCallback = new Runnable(){
     *      //处理线程逻辑
     * }
     * Thread producer = new Thread(producerCallback)
     * producer.start()
     */
    pthread_create(&producer, NULL, producerCallback, NULL);
    pthread_create(&consumer, NULL, consumerCallback, NULL);
}

/*
 * C++主线程调用Java方法
 * 1.根据object获取jclass（若调用静态方法则跳过这一步）
 * 如：jclass clz = env->GetObjectClass(jobj)
 * 2.获取jmethodid
 * 如：jmethodid jmid = env->GetMethodId(clz, "funName", "(ILJava/lang/String;)V")
 * 3.调用方法
 * 如：jenv->CallVoidMethod(jobj, jmid, code, jmsg)
 *
 * C++子线程调用Java方法
 * 1.获取JVM对象：JNI_OnLoad(JavaVM* vm, void* reserved)
 * 2.通过JVM获取JniEnv：
 * JNIEnv *env
 * jvm->AttachCurrentThread(&env, 0)
 * //调用Java方法
 * jvm->DetachCurrentThread()
 * */
JavaVM *javaVm = NULL;
WlCallJava *callJava = NULL;
WlFFmpeg *fFmpeg = NULL;
WlPlayStatus *status = NULL;
pthread_t threadStart;

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    JNIEnv *env;
    javaVm = vm;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGD("初始化JVM失败")
        return result;
    }
    LOGD("初始化JVM成功")
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_prepareNative(JNIEnv *env,
                                                              jobject thiz,
                                                              jstring source1) {
    const char *source = env->GetStringUTFChars(source1, 0);
    if (fFmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new WlCallJava(javaVm, env, &thiz);
        }
        status = new WlPlayStatus();
        fFmpeg = new WlFFmpeg(status, callJava, source);
        fFmpeg->prepared();
    }
//    env->ReleaseStringUTFChars(source1, source);

}

void *startCallback(void *data){
    WlFFmpeg *wlFFmpeg = static_cast<WlFFmpeg *>(data);
    wlFFmpeg->start();
    pthread_exit(&threadStart);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_startNative(JNIEnv *env,
                                                            jobject thiz) {

    if (fFmpeg != NULL) {
        pthread_create(&threadStart, NULL, startCallback, fFmpeg);
    }
}

bool nexit = true;

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_pauseNative(JNIEnv *env,
                                                            jobject thiz) {
    if(fFmpeg != NULL){
        fFmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_resumeNative(JNIEnv *env,
                                                             jobject thiz) {
    if(fFmpeg != NULL){
        fFmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_stopNative(JNIEnv *env,
                                                           jobject thiz) {
    if(!nexit){
        return;
    }

    jclass jlz = env->GetObjectClass(thiz);
    jmethodID jmidPlayNext = env->GetMethodID(jlz, "onCallNext", "()V");

    nexit = false;
    if(fFmpeg != NULL){
        fFmpeg->release();
        delete(fFmpeg);
        fFmpeg = NULL;
        if(callJava != NULL){
            delete(callJava);
            callJava = NULL;
        }
        if(status != NULL){
            delete(status);
            status = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(thiz, jmidPlayNext);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_seekNative(JNIEnv *env,
                                                           jobject thiz,
                                                           jint secds) {
    if(fFmpeg != NULL){
        fFmpeg->seek(secds);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_setVolumeNative(JNIEnv *env,
                                                                jobject thiz,
                                                                jint percent) {
    if(fFmpeg==NULL) return;
    fFmpeg->setVolume(percent);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_setPitchNative(JNIEnv *env,
                                                               jobject thiz,
                                                               jfloat pitch) {
    if(fFmpeg==NULL) return;
    fFmpeg->setPitch(pitch);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_setTempoNative(JNIEnv *env,
                                                               jobject thiz,
                                                               jfloat tempo) {
    if(fFmpeg==NULL) return;
    fFmpeg->setTempo(tempo);
}

//// 引擎接口
//SLObjectItf engineObject = NULL;
//SLEngineItf engineEngine = NULL;
//
////混音器
//SLObjectItf outputMixObject = NULL;
//SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
//SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
//
//
////pcm
//SLObjectItf pcmPlayerObject = NULL;
//SLPlayItf pcmPlayerPlay = NULL;
//SLVolumeItf pcmPlayerVolume = NULL;
//
////缓冲器队列接口
//SLAndroidSimpleBufferQueueItf pcmBufferQueue;
//
//FILE *pcmFile;
//void *buffer;
//
//uint8_t *out_buffer;
//
//int getPcmData(void **pcm)
//{
//    int size = 0;
//    while(!feof(pcmFile))
//    {
//        size = fread(out_buffer, 1, 44100 * 2 * 2, pcmFile);
//        if(out_buffer == NULL)
//        {
//            LOGI("%s", "read end");
//            break;
//        } else{
//            LOGI("%s", "reading");
//        }
//        *pcm = out_buffer;
//        break;
//    }
//    return size;
//}

//void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context)
//{
//    //assert(NULL == context);
//    int size = getPcmData(&buffer);
//    // for streaming playback, replace this test by logic to find and fill the next buffer
//    if (NULL != buffer) {
//        SLresult result;
//        // enqueue another buffer
//        result = (*pcmBufferQueue)->Enqueue(pcmBufferQueue, buffer, size);
//        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
//        // which for this code example would indicate a programming error
//    }
//}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_audioplayer_player_AudioPlayer_playPCM(JNIEnv *env, jobject instance,
                                                        jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

//    //读取pcm文件
//    pcmFile = fopen(url, "r");
//    if(pcmFile == NULL)
//    {
//        LOGE("%s", "fopen file error");
//        return;
//    }
//    out_buffer = (uint8_t *) malloc(44100 * 2 * 2);
//
//
//    SLresult result;
//    //第一步------------------------------------------
//    // 创建引擎对象
//    slCreateEngine(&engineObject, 0, 0, 0, 0, 0); //创建引擎
//    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE); //实现engineObject接口对象
//    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine); //通过engineObject方法初始化engine
//
//
//    //第二步-------------------------------------------
//    // 创建混音器
//    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
//    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
//    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
//    (void)result;
//    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
//    (void)result;
//    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
//    if (SL_RESULT_SUCCESS == result) {
//        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
//                outputMixEnvironmentalReverb, &reverbSettings);
//        (void)result;
//    }
//    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
//
//    // 第三步--------------------------------------------
//    // 创建播放器
//    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
//    SLDataFormat_PCM pcm={
//            SL_DATAFORMAT_PCM,//播放pcm格式的数据
//            2,//2个声道（立体声）
//            SL_SAMPLINGRATE_44_1,//44100hz的频率
//            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
//            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
//            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
//            SL_BYTEORDER_LITTLEENDIAN//结束标志
//    };
//
//    SLDataSource slDataSource = {&android_queue, &pcm};
//    SLDataSink audioSnk = {&outputMix, NULL};
//    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
//    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
//
//    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 3, ids, req);
//    // 初始化播放器
//    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
//
//    //得到接口后调用  获取Player接口
//    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
//
//    //第四步---------------------------------------
//    // 创建缓冲区和回调函数
//    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
//
//    //缓冲接口回调
//    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, NULL);
//    //获取音量接口
//    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmPlayerVolume);
//
//    //第五步----------------------------------------
//    // 设置播放状态
//    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
//
//
//    //第六步----------------------------------------
//    // 主动调用回调函数开始工作
//    pcmBufferCallBack(pcmBufferQueue, NULL);

    env->ReleaseStringUTFChars(url_, url);
}