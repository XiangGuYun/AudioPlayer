//
// Created by yangw on 2018-2-28.
//

#include "WlAudio.h"

WlAudio::WlAudio(WlPlayStatus *playstatus, int sample_rate, WlCallJava *callJava) {
    this->playstatus = playstatus;
    this->sample_rate = sample_rate;
    this->callJava = callJava;
    queue = new WlQueue(playstatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);
    sampleType = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(1.0f);
    soundTouch->setTempo(1.0f);
}

WlAudio::~WlAudio() {

}

int WlAudio::getSoundTouchData() {
    while (playstatus != NULL && !playstatus->exit) {
        out_buffer = NULL;
        if (finished) {
            finished = false;
            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
            if (data_size > 0) {
                for (int i = 0; i < data_size / 2 + 1; ++i) {
                    sampleType[i] = (out_buffer[i * 2] | ((out_buffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampleType, nb);
                num = soundTouch->receiveSamples(sampleType, data_size / 4);
            } else {
                soundTouch->flush();
            }
        }
        if(num == 0){
            finished = true;
            continue;
        } else {
            if(out_buffer == NULL){
                num = soundTouch->receiveSamples(sampleType, data_size/4);
                if(num == 0){
                    finished = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}

void *decodPlay(void *data) {
    WlAudio *wlAudio = (WlAudio *) data;

    wlAudio->initOpenSLES();

    pthread_exit(&wlAudio->thread_play);
}

void WlAudio::play() {
    LOGD("创建了播放线程")
    pthread_create(&thread_play, NULL, decodPlay, this);

}

int WlAudio::resampleAudio(void **pcmBuf) {

    while (playstatus != NULL && !playstatus->exit) {
        if (queue->size() == 0) {
            if (!playstatus->load) {
                playstatus->load = true;
                callJava->onCallLoaded(CHILD_THREAD, true);
            }
            continue;
        } else {
            if (playstatus->load) {
                playstatus->load = false;
                callJava->onCallLoaded(CHILD_THREAD, false);
            }
        }

        avPacket = av_packet_alloc();
        if (queue->pop(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0) {

            if (avFrame->channels && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swr_ctx;

            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    NULL, NULL
            );
            if (!swr_ctx || swr_init(swr_ctx) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swr_ctx);
                continue;
            }

            nb = swr_convert(
                    swr_ctx,
                    &buffer,
                    avFrame->nb_samples,
                    (const uint8_t **) avFrame->data,
                    avFrame->nb_samples);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            LOGE("data_size is %d", data_size);

            now_time = avFrame->pts * av_q2d(time_base);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = static_cast<int>(now_time);
            *pcmBuf = buffer;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            break;
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
    return data_size;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    WlAudio *wlAudio = (WlAudio *) context;
    if (wlAudio != NULL) {
        int buffersize = wlAudio->getSoundTouchData();
        if (buffersize > 0) {
            wlAudio->clock += buffersize / ((double) (wlAudio->sample_rate * 2 * 2));
            if (wlAudio->clock - wlAudio->lastTime >= 0.1) {
                wlAudio->lastTime = wlAudio->clock;
                wlAudio->callJava->onCallProgress(CHILD_THREAD, wlAudio->clock, wlAudio->duration);
            }
            wlAudio->callJava->onCallVolumeDB(CHILD_THREAD, wlAudio->getPcmDB(
                    reinterpret_cast<char *>(wlAudio->sampleType), buffersize * 4));
            (*wlAudio->pcmBufferQueue)->Enqueue(wlAudio->pcmBufferQueue, (char *) wlAudio->sampleType,
                                                buffersize * 2 * 2);
        }
    }
}

void WlAudio::initOpenSLES() {
    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            static_cast<SLuint32>(getCurrentSampleRateForOpensles(sample_rate)),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 2,
                                       ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    //初始化音量控制器
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);
//    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
//    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);
}

int WlAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void WlAudio::pause() {
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

void WlAudio::resume() {
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void WlAudio::stop() {
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}

void WlAudio::release() {
    stop();
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if (playstatus != NULL) {
        playstatus = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
}

void WlAudio::setVolume(int percent) {
    if (pcmVolumePlay == NULL) return;
    if (percent != 0) {
        (*pcmVolumePlay)->SetMute(pcmVolumePlay, SL_BOOLEAN_FALSE);
        (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -50);
    } else {
        (*pcmVolumePlay)->SetMute(pcmVolumePlay, SL_BOOLEAN_TRUE);
    }
}

void WlAudio::setPitch(float pitch) {
    soundTouch->setPitch(pitch);
}

void WlAudio::setTempo(float tempo) {
    soundTouch->setTempo(tempo);
}

int WlAudio::getPcmDB(char *pcmData, size_t pcmSize) {
    int db = 0;
    short int per16BitValue = 0;
    double sumValue = 0;
    for (int i = 0; i < pcmSize; i+=2) {
        memcpy(&per16BitValue, pcmData+i, 2);
        sumValue += abs(per16BitValue);
    }
    sumValue = sumValue/(pcmSize/2);
    if(sumValue>0){
        db = (int)20.0 * log10(sumValue);
    }
    return db;
}
