package com.android.audioplayer.thread_demo

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.android.audioplayer.R
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val threadDemo = ThreadDemo()
        //开启普通线程
        btnCreateNormalThread.setOnClickListener {
            threadDemo.normalThread()
        }
        //开启互斥线程
        btnCreateMutexThread.setOnClickListener {
            threadDemo.mutexThread()
        }
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
    }
}
