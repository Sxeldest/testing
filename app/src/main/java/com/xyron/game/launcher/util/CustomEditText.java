package com.xyron.game.launcher.util;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.widget.EditText;

import com.xyron.game.main.SAMP;

@SuppressLint("AppCompatCustomView")
public class CustomEditText extends EditText {
    private Context mContext = null;

    public CustomEditText(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    public void SetBackListener(Context context) {
        mContext = context;
    }

    @Override
    protected void onSelectionChanged(int selStart, int selEnd) {
        super.onSelectionChanged(selStart, selEnd);
        if (mContext instanceof SAMP) {
            ((SAMP) mContext).onCursorChanged(selStart, selEnd);
        }
    }

    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            ((SAMP)mContext).onEventBackPressed();
            return true;
        }
        return false;
    }
}
