package com.android.audioplayer.player

import android.app.Activity
import com.android.audioplayer.player.listener.*

class AudioPlayer(val ctx: Activity) {
    //音频源
    lateinit var source: String
    //准备完成监听器
    var listener: WlOnPreparedListener? = null
    //加载完成监听器
    var loadListener: WlOnLoadListener? = null
    //播放状态监听器
    var playStateListener: WlPlayStateListener? = null
    //播放进度监听器
    var playingListener: WlOnPlayingListener? = null
    //错误监听器
    var errorListener: WlOnErrorListener? = null
    //完成监听器
    var completeListener: WlOnCompleteListener? = null
    //振幅监听器
    var volumeDBListener: WlOnVolumeDBListener? = null
    var timeInfo: TimeInfo? = null
    //是否停止播放
    var stopped = false
    //是否处于暂停播放
    var paused = false

    /**
     * 监听准备事件
     */
    fun listenPrepared(callback: () -> Unit){
        setWlOnPreparedListener(object :
            WlOnPreparedListener {
            override fun onPrepared() {
                ctx.runOnUiThread {
                    callback.invoke()
                }
            }
        })
    }

    /**
     * 监听加载事件
     */
    fun listenLoaded(callback: (load: Boolean) -> Unit){
        setWlOnLoadListener(object :
            WlOnLoadListener {
            override fun onLoad(load: Boolean) {
                ctx.runOnUiThread {
                    callback.invoke(load)
                }
            }
        })
    }

    /**
     * 监听播放状态
     */
    fun listenPlayState(callback: (pause: Boolean) -> Unit){
        setWlPlayStateListener(object :
            WlPlayStateListener {
            override fun onPause(pause: Boolean) {
                ctx.runOnUiThread {
                    callback.invoke(pause)
                }
            }
        })
    }

    inner class TimeInfo(var currentTime:Int = 0, var totalTime:Int = 0)

    /**
     * 监听播放过程
     */
    fun listenPlaying(callback: (info: TimeInfo) -> Unit){
        setWlPlayingListener(object :WlOnPlayingListener{
            override fun progress(info: TimeInfo) {
                ctx.runOnUiThread {
                    callback.invoke(info)
                }
            }
        })
    }

    /**
     * 监听播放错误
     */
    fun listenError(callback: (code: Int, msg: String) -> Unit){
        setOnWlOnErrorListener(object :
            WlOnErrorListener {
            override fun onError(code: Int, msg: String) {
                ctx.runOnUiThread {
                    callback.invoke(code, msg)
                }
            }
        })
    }

    /**
     * 监听播放完成
     */
    fun listenCompleted(callback: () -> Unit){
        setOnCompleteListener(object :
            WlOnCompleteListener {
            override fun onComplete() {
                ctx.runOnUiThread {
                    callback.invoke()
                }
            }
        })
    }

    /**
     * 监听音量分贝
     */
    fun listenVolumeDb(callback:(db:Int)->Unit){
        setOnWlOnVolumeDbListener(object :
            WlOnVolumeDBListener {
            override fun onDbValue(db: Int) {
                ctx.runOnUiThread {
                    callback.invoke(db)
                }
            }
        })
    }

    /**
     * 准备播放
     */
    fun prepare() {
        if (source.isEmpty()) return
        onCallLoaded(true)
        Thread {
            prepareNative(source)
        }.start()
    }

    /**
     * 切换播放位置
     */
    fun seek(secds: Int) {
        seekNative(secds)
    }

    /**
     * 开始播放
     */
    fun start(startCallback:()->Unit) {
        if (source.isEmpty()) return
        stopped = false
        Thread {
            startNative()
            startCallback.invoke()
        }.start()
    }

    /**
     * 暂停播放
     */
    fun pause() {
        paused = true
        pauseNative()
        playStateListener?.onPause(true)
    }

    /**
     * 恢复播放
     */
    fun resume() {
        paused = false
        resumeNative()
        playStateListener?.onPause(false)
    }

    /**
     * 停止播放
     */
    fun stop() {
        if (source.isEmpty()) {
            return
        }
        stopped = true
        Thread {
            stopNative()
        }.start()
    }

    /**
     * 切换播放
     */
    fun playNext(url: String) {
        source = url
        playNext = true
        stop()
    }

    /**
     * 设置音量
     */
    fun setVolume(percent: Int) {
        if (percent in 0..100)
            setVolumeNative(percent)
    }

    /**
     * 设置音调
     */
    fun setPitch(pitch: Float){
        if(pitch in 0.1..2.0){
            setPitchNative(pitch)
        }
    }

    /**
     * 设置播放速度
     */
    fun setTempo(tempo: Float){
        if(tempo in 0.1..2.0){
            setTempoNative(tempo)
        }
    }

    /** 准备播放 */
    private external fun prepareNative(source: String)
    /** 开始播放 */
    private external fun startNative()
    /** 播放PCM文件 */
    private external fun playPCM(file: String)
    /** 暂停播放 */
    private external fun pauseNative()
    /** 恢复播放 */
    private external fun resumeNative()
    /** 停止播放 */
    private external fun stopNative()
    /** 改变进度 */
    private external fun seekNative(secds: Int)
    /** 设置音量 */
    private external fun setVolumeNative(percent: Int)
    /** 设置音调 */
    private external fun setPitchNative(pitch: Float)
    /** 设置速度 */
    private external fun setTempoNative(tempo: Float)

    private fun setWlOnPreparedListener(listener: WlOnPreparedListener) {
        this.listener = listener
    }

    private fun setWlOnLoadListener(listener: WlOnLoadListener) {
        this.loadListener = listener
    }

    private fun setWlPlayStateListener(listener: WlPlayStateListener) {
        this.playStateListener = listener
    }

    private fun setWlPlayingListener(listener: WlOnPlayingListener) {
        this.playingListener = listener
    }

    private fun setOnWlOnErrorListener(listener: WlOnErrorListener) {
        this.errorListener = listener
    }

    private fun setOnCompleteListener(listener: WlOnCompleteListener) {
        this.completeListener = listener
    }

    private fun setOnWlOnVolumeDbListener(listener: WlOnVolumeDBListener){
        this.volumeDBListener = listener
    }

    /**
     * 此方法提供给C++调用，表示准备工作已完成，通知Java层可以开始播放了
     */
    private fun onCallPrepared() {
        listener?.onPrepared()
    }

    /**
     * 此方法提供给C++调用
     */
    private fun onCallLoaded(load: Boolean) {
        loadListener?.onLoad(load)
    }

    /**
     * 此方法提供给C++调用
     */
    private fun onCallNext() {
        if (playNext) {
            playNext = false
            prepare()
        }
    }

    /**
     * 此方法提供给C++调用
     */
    private fun error(code: Int, msg: String) {
        stop()
        errorListener?.onError(code, msg)
    }

    /**
     * 此方法提供给C++调用
     */
    private fun onComplete() {
        stop()
        completeListener?.onComplete()
    }

    /**
     * 此方法提供给C++调用
     */
    private fun progress(currentTime: Int, totalTime: Int) {
        synchronized(this) {
            if (timeInfo == null) timeInfo = TimeInfo()
            timeInfo?.currentTime = currentTime
            timeInfo?.totalTime = totalTime
            timeInfo?.let { playingListener?.progress(it) }
        }
    }

    /**
     * 此方法提供给C++调用
     */
    private fun onCallVolumeDB(db: Int){
        volumeDBListener?.onDbValue(db)
    }

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

        var playNext = false
    }
}