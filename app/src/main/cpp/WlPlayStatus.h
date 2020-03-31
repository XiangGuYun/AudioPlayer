//
// Created by 86139 on 2020/3/26.
//

#ifndef AUDIOPLAYER_WlPlayStatus_H
#define AUDIOPLAYER_WlPlayStatus_H


class WlPlayStatus {
public:
    bool exit;
    bool load = true;
    bool seek = false;
public:
    WlPlayStatus();
    ~WlPlayStatus();
};


#endif //AUDIOPLAYER_WlPlayStatus_H
