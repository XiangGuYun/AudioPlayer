package com.android.audioplayer.multithread_decode_audio

import android.widget.Toast

class WlPlayer {
    //音频源
    lateinit var source: String
    //准备完成监听器
    var listener: WlOnPreparedListener? = null
    //加载完成监听器
    var loadListener: WlOnLoadListener? = null
    //播放状态监听器
    var playStateListener: WlPlayStateListener? = null
    //播放进度监听器
    var playingListener:WlOnPlayingListener? = null
    //错误监听器
    var errorListener:WlOnErrorListener? = null
    //完成监听器
    var completeListener:WlOnCompleteListener? = null
    var timeInfo:TimeInfo? = null
    var stopped = false

    /**
     * 准备完成监听器
     */
    fun setWlOnPreparedListener(listener: WlOnPreparedListener) {
        this.listener = listener
    }

    fun setWlOnLoadListener(listener: WlOnLoadListener) {
        this.loadListener = listener
    }

    fun setWlPlayStateListener(listener: WlPlayStateListener){
        this.playStateListener = listener
    }

    fun setWlPlayingListener(listener: WlOnPlayingListener){
        this.playingListener = listener
    }

    fun setOnWlOnErrorListener(listener: WlOnErrorListener){
        this.errorListener = listener
    }

    fun setOnCompleteListener(listener: WlOnCompleteListener){
        this.completeListener = listener
    }

    /**
     * 此方法提供给C++调用，表示准备工作已完成，通知Java层可以开始播放了
     */
    fun onCallPrepared() {
        listener?.onPrepared()
    }

    /**
     * 此方法提供给C++调用
     */
    fun onCallLoaded(load:Boolean){
        loadListener?.onLoad(load)
    }

    /**
     * 此方法提供给C++调用
     */
    fun pause(){
        pauseNative()
        playStateListener?.onPause(true)
    }

    /**
     * 此方法提供给C++调用
     */
    fun resume(){
        resumeNative()
        playStateListener?.onPause(false)
    }

    /**
     * 此方法提供给C++调用
     */
    fun error(code:Int, msg:String){
        stop()
        errorListener?.onError(code, msg)
    }

    fun onComplete(){
        stop()
        completeListener?.onComplete()
    }

    fun progress(currentTime:Int, totalTime:Int){
        synchronized(this){
            if(timeInfo == null) timeInfo = TimeInfo()
            timeInfo?.currentTime = currentTime
            timeInfo?.totalTime = totalTime
            timeInfo?.let { playingListener?.progress(it) }
        }

    }

    /**
     * 准备工作
     */
    fun prepare() {
        if (source.isEmpty()) return
        onCallLoaded(true)
        Thread {
            prepareNative(source)
        }.start()
    }

    fun seek(secds:Int){
        seekNative(secds)
    }

    /**
     * 开始播放
     */
    fun start() {
        if (source.isEmpty()) return
        stopped = false
        Thread {
            startNative()
        }.start()
    }

    fun stop() {
        if (source.isEmpty()){
            return
        }
        stopped = true
        Thread{
            stopNative()
        }.start()
    }

    private external fun prepareNative(source: String)
    private external fun startNative();
    private external fun playPCM(file: String);
    private external fun pauseNative();
    private external fun resumeNative();
    private external fun stopNative();
    private external fun seekNative(secds:Int);

    companion object {
        init {
            System.loadLibrary("native-lib")
            System.loadLibrary("avcodec-57")
            System.loadLibrary("avdevice-57")
            System.loadLibrary("avfilter-6")
            System.loadLibrary("avformat-57")
            System.loadLibrary("avutil-55")
            System.loadLibrary("postproc-54")
            System.loadLibrary("swresample-2")
            System.loadLibrary("swscale-4")
        }
    }
}