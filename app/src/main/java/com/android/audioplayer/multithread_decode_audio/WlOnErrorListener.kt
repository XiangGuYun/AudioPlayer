package com.android.audioplayer.multithread_decode_audio

interface WlOnErrorListener {
    fun onError(code:Int, msg:String)
}