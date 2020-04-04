package com.android.audioplayer.multithread_decode_audio;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;

import java.util.ArrayList;

public class RainView extends BaseView {

    ArrayList<RainItem> rainList = new ArrayList<RainItem>();
    int size = 80;

    public RainView(Context context) {
        super(context);
    }

    public RainView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void init() {
        for (int i = 0; i < size; i++) {
            RainItem item = new RainItem(getWidth(), getHeight());
            rainList.add(item);
        }
    }

    @Override
    protected void logic() {
        for (RainItem item : rainList) {
            item.move();
        }
    }

    @Override
    protected void drawSub(Canvas canvas) {
        // drawSub 完成绘制操作
        for (RainItem item : rainList) {
            item.draw(canvas);
        }
    }

}