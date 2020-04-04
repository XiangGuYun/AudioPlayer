package com.android.audioplayer.view;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.View;

public abstract class BaseView extends View {

    Thread thread;
    
    public BaseView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public BaseView(Context context) {
        super(context);
    }

    @Override
    final protected void onDraw(Canvas canvas) {
        //禁止子类覆盖，用final
        if(thread == null ) {
            thread = new MyThread();
            thread.start();
        } else{
            drawSub(canvas);
        }
    }

    protected abstract void init();
    protected abstract void logic();
    protected abstract void drawSub(Canvas canvas);

    @Override
    final protected void onDetachedFromWindow() {
        // 离开屏幕时结束
        //onDetachedFromWindow在销毁资源（既销毁view）之后调用
        running = false;
        super.onDetachedFromWindow();
    }
    private boolean running = true;
    class MyThread extends Thread {
        @Override
        public void run() {
            init();
            while(running) {
                logic();
                postInvalidate(); //线程中更新绘制，重新调用onDraw方法
                try {
                    Thread.sleep(50); //速度太快肉眼看不到，要睡眠
                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

}