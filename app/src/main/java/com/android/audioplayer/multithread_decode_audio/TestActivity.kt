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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        supportActionBar?.hide()
        setContentView(R.layout.activity_test)
        player = WlPlayer()
        player.source = "http://sharefs.yun.kugou.com/202004022320/5074dee4cb9bddef14be3362e330cc1b/G006/M06/0B/15/Rg0DAFS33u2AFo9_AD4QE6gg03M473.mp3"
//        player.source = "/storage/emulated/0/Pictures/天空之城.mp3"

        player.setWlOnLoadListener(object : WlOnLoadListener {
            override fun onLoad(load: Boolean) {
                Log.d("Test", if (load) "加载中" else "播放中")
            }
        })

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

        player.setWlOnPreparedListener(object : WlOnPreparedListener {
            override fun onPrepared() {
                Log.d("Test", "onPrepared...")
                playing = true
                player.start()
            }
        })

        player.setWlPlayStateListener(object : WlPlayStateListener {
            override fun onPause(pause: Boolean) {
                Log.d("Test", if (pause) "暂停中" else "播放中")
            }
        })

        seekBar.setOnSeekBarChangeListener(object :SeekBar.OnSeekBarChangeListener{
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                Log.d("Test", "progress is $progress")
                if(fromUser){
                    player.seek(progress)
                }
            }
            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }
            override fun onStopTrackingTouch(seekBar: SeekBar?) {

            }
        })

        val sdf = SimpleDateFormat("mm:ss")
        player.setWlPlayingListener(object : WlOnPlayingListener {
            override fun progress(info: TimeInfo) {
                Log.d("Test", info.toString())
                runOnUiThread {
                    tvTime.text = "${sdf.format(info.currentTime * 1000)}/${sdf.format(info.totalTime * 1000)}"
                    seekBar.max = info.totalTime
                    seekBar.progress = info.currentTime
                }
            }
        })

        player.setOnWlOnErrorListener(object : WlOnErrorListener {
            override fun onError(code: Int, msg: String) {
                runOnUiThread {
                    Toast.makeText(this@TestActivity, "code is $code, msg is $msg", Toast.LENGTH_SHORT).show()
                }
            }
        })

        player.setOnCompleteListener(object :WlOnCompleteListener{
            override fun onComplete() {
                runOnUiThread {
                    Toast.makeText(this@TestActivity, "播放完成了", Toast.LENGTH_SHORT).show()
                }
            }
        })

        btnStop.setOnClickListener {
            tvTime.text = "00:00/00:00"
            seekBar.progress = 0
            player.stop()
        }

    }

    override fun onDestroy() {
        super.onDestroy()
        if(!player.stopped)
            player.stop()
    }


}
