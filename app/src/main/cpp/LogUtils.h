//
// Created by 86139 on 2020/3/26.
//

#ifndef AUDIOPLAYER_LOGUTILS_H
#define AUDIOPLAYER_LOGUTILS_H

#include <android/log.h>


#define LOG_DEBUG true
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ywl5320",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG,"ywl5320",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ywl5320",FORMAT,##__VA_ARGS__);

#endif //AUDIOPLAYER_LOGUTILS_H
