package com.android.audioplayer.player.listener

import com.android.audioplayer.player.AudioPlayer


interface WlOnPlayingListener {
    fun progress(info: AudioPlayer.TimeInfo)
}