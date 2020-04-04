package com.android.audioplayer.multithread_decode_audio

import android.Manifest.permission.READ_EXTERNAL_STORAGE
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.View
import android.widget.SeekBar
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.android.audioplayer.R
import com.tbruyelle.rxpermissions2.RxPermissions
import kotlinx.android.synthetic.main.activity_test.*
import java.io.File
import java.text.SimpleDateFormat

class TestActivity : AppCompatActivity() {

    lateinit var player: WlPlayer
    var playing = false
    val songList = arrayListOf(
        "海阔天空" to "http://sharefs.yun.kugou.com/202004040258/9a9147e70068345b0e197636e42039b1/G172/M05/14/0D/7A0DAF1xXbmAIZOSADIEJ1SxOn0675.mp3",
        "那个女孩" to "http://sharefs.yun.kugou.com/202004040154/a2365114898251f25f3435a4769b39e2/G136/M04/1B/05/aJQEAFt2Qh6Adz96ADXZQ06lVQA689.mp3",
        "那女孩对我说" to "http://sharefs.yun.kugou.com/202004040157/1368fe4ee30dbf168e459e00aef94b78/G012/M07/15/19/rIYBAFT7ycCAHnzYAEIzAFFAo1Y787.mp3",
        "美人鱼" to "http://sharefs.yun.kugou.com/202004040221/1e8980fd735a56c4591fdb9c846d04c0/G006/M06/0B/15/Rg0DAFS33u2AFo9_AD4QE6gg03M473.mp3"
    )
    var currentSongIndex = 0
    lateinit var timeInfo: TimeInfo
    var currentTempo = 20
    var currentPitch = 20
    var currentVolume = 100

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        supportActionBar?.hide()
        setContentView(R.layout.activity_test)
        player = WlPlayer(this)
        player.source = songList[0].second
//        player.source = "/storage/emulated/0/Pictures/天空之城.mp3"

        player.listenLoaded { load ->
            Log.d("Test", if (load) "加载中" else "播放中")
        }

        btnPause.setOnClickListener {
            if (playing) {
                //暂停
                player.pause()
            } else {
                //继续
                player.resume()
            }
            playing = !playing
        }

        btnBegin.setOnClickListener {
            RxPermissions(this).requestEachCombined(WRITE_EXTERNAL_STORAGE, READ_EXTERNAL_STORAGE)
                .subscribe { permission ->
                    when {
                        permission.granted -> {
                            player.prepare()
                        }
                    }
                }
        }

        player.listenPrepared {
            Log.d("Test", "onPrepared...")
            tvSongName.text = songList[currentSongIndex].first
            playing = true
            player.start {
                player.setPitch(currentPitch / 20f)
                player.setTempo(currentTempo / 20f)
            }
        }

        player.listenPlayState { pause ->
            Log.d("Test", if (pause) "暂停中" else "播放中")
        }

        seekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                Log.d("Test", "progress is $progress")
                if (fromUser) {
                    player.seek(progress)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {

            }
        })

        sbVolume.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                Log.d("Test", "progress is $progress")
                if (fromUser) {
                    if (!player.paused) player.pause()
                    currentVolume = progress
                    player.setVolume(0)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                player.resume()
                player.setVolume(currentPitch)
            }
        })

        sbPitch.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                Log.d("Test", "progress is $progress")
                if (fromUser) {
                    currentPitch = progress
                    player.setPitch(progress / 20f)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {

            }
        })

        sbTempo.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                Log.d("Test", "progress is $progress")
                if (fromUser) {
                    currentTempo = progress
                    player.setTempo(progress / 20f)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {

            }
        })

        val sdf = SimpleDateFormat("mm:ss")
        player.listenPlaying { info ->
            timeInfo = info
            tvTime.text =
                "${sdf.format(info.currentTime * 1000)}/${sdf.format(info.totalTime * 1000)}"
            seekBar.max = info.totalTime
            seekBar.progress = info.currentTime
        }

        player.listenError { code, msg ->
            Toast.makeText(
                this@TestActivity,
                "code is $code, msg is $msg",
                Toast.LENGTH_SHORT
            ).show()
        }

        player.listenCompleted {
            seekBar.progress = 0
            tvTime.text = "00:00/${sdf.format(timeInfo.totalTime * 1000)}"
        }

        player.listenVolumeDb { db ->
            Log.d("Test", "db is $db")
        }

        btnStop.setOnClickListener {
            tvTime.text = "00:00/00:00"
            seekBar.progress = 0
            player.stop()
        }

        btnPre.setOnClickListener {
            if (currentSongIndex == 0) return@setOnClickListener
            player.playNext(songList[--currentSongIndex].second)
            tvSongName.text = songList[currentSongIndex].first
        }

        btnNext.setOnClickListener {
            if (currentSongIndex == songList.size - 1) return@setOnClickListener
            player.playNext(songList[++currentSongIndex].second)
            tvSongName.text = songList[currentSongIndex].first
        }

    }

    override fun onDestroy() {
        super.onDestroy()
        if (!player.stopped)
            player.stop()
    }


}
