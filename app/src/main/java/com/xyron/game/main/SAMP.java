package com.xyron.game.main;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.Toast;
import androidx.constraintlayout.widget.ConstraintLayout;
import com.google.firebase.crashlytics.FirebaseCrashlytics;
import com.xyron.game.R;
import com.xyron.game.main.ui.AttachEdit;
import com.xyron.game.main.ui.CustomKeyboard;
import java.io.File;
import java.nio.charset.StandardCharsets;
import java.util.Locale;
import com.xyron.game.launcher.util.ConfigValidator;
import org.ini4j.Wini;

public class SAMP extends com.xyron.game.main.GTASA implements CustomKeyboard.InputListener, HeightProvider.HeightListener {
    private static final String TAG = "SAMP";
    public static final String EXTRA_NICKNAME = "nickname";
    private static SAMP instance;

    public native void initializeSAMP();
    public native void setAllowNextNativePauseMenu(boolean allow);

    private CustomKeyboard mKeyboard;
    private HeightProvider mHeightProvider;
    private AttachEdit mAttachEdit;

    private ConstraintLayout hud_main;
    private ConstraintLayout loadingscreen;
    private int iShowHud = 0;

    static {
        Log.i(TAG, "**** Loading SO's");
        try {
            System.loadLibrary("bass");
            com.bytedance.shadowhook.ShadowHook.init(
                    new com.bytedance.shadowhook.ShadowHook.ConfigBuilder()
                            .setMode(com.bytedance.shadowhook.ShadowHook.Mode.SHARED)
                            .setDebuggable(true)
                            .setRecordable(true)
                            .build()
            );
            System.loadLibrary("SAMP");
        } catch (Throwable e) {
            Log.e(TAG, "Library load failed", e);
        }
    }

    public native void sendDialogResponse(int i, int i2, int i3, byte[] str);
    public static SAMP getInstance() { return instance; }

    // Engine Callbacks
    public void showTab() {}
    public void hideTab() {}
    public void setTab(int id, String name, int score, int ping) {}
    public void clearTab() {}
    public void ShowLogo(boolean show) {}

    public void showLoadingScreen() {
        runOnUiThread(() -> { if (loadingscreen != null) loadingscreen.setVisibility(View.VISIBLE); });
    }

    public void hideLoadingScreen() {
        runOnUiThread(() -> {
            if (loadingscreen != null) loadingscreen.setVisibility(View.GONE);
            MostrarChat();
        });
    }

    public void setPauseState(boolean pause) {
        runOnUiThread(() -> { if (pause) hideSystemUI(); else showSystemUI(); });
    }

    public void exitGame() {
        FirebaseCrashlytics.getInstance().setCrashlyticsCollectionEnabled(false);
        finishAndRemoveTask();
        System.exit(0);
    }

    public void handleConnectionFailure(String host, int port, int attempts) {
        runOnUiThread(() -> {
            String address = (host == null || host.trim().isEmpty()) ? "servidor" : host + ":" + port;
            Toast.makeText(this, "Conexao falhou: " + address, Toast.LENGTH_LONG).show();
            startActivity(new Intent(this, com.xyron.game.launcher.EntryActivity.class).addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP));
            finish();
        });
    }

    public void hideWithoutReset() {
        runOnUiThread(() -> { if (hud_main != null) hud_main.setVisibility(View.GONE); mAttachEdit.hideWithoutReset(); });
    }

    public void showWithoutReset() {
        runOnUiThread(() -> { if (iShowHud == 1 && hud_main != null) hud_main.setVisibility(View.VISIBLE); if (mAttachEdit.isShow) mAttachEdit.showWithoutReset(); });
    }

    public void showEditObject() { runOnUiThread(() -> mAttachEdit.show()); }
    public void hideEditObject() { runOnUiThread(() -> mAttachEdit.hide()); }

    @Override
    public void OnInputEnd(String str) {
        byte[] b;
        try { b = str.getBytes("windows-1251"); } catch (Exception e) { b = str.getBytes(); }
        onInputEnd(b);
    }
    public native void onInputEnd(byte[] str);

    @Override
    public void OnInputUpdate(String str) {
        byte[] b_upd;
        try { b_upd = str.getBytes("windows-1251"); } catch (Exception e) { b_upd = str.getBytes(); }
        onInputUpdate(b_upd);
    }
    public native void onInputUpdate(byte[] str);

    @Override
    public void onCursorChanged(int start, int end) {
        onCursorChangedNative(start, end);
    }
    public native void onCursorChangedNative(int start, int end);

    @Override
    public void onKeyStroke(int keyCode, int metaState) {
        onKeyStrokeNative(keyCode, metaState);
    }
    public native void onKeyStrokeNative(int keyCode, int metaState);

    public void showKeyboard() { runOnUiThread(() -> mKeyboard.ShowInputLayout()); }
    public void hideKeyboard() { runOnUiThread(() -> mKeyboard.HideInputLayout()); }

    public void setKeyboardText(String text) {
        if (mKeyboard != null) mKeyboard.setText(text);
    }

    public void setKeyboardSelection(int start, int end) {
        if (mKeyboard != null) mKeyboard.setSelection(start, end);
    }

    public boolean isKeyboardShowing() {
        if (mKeyboard != null) return mKeyboard.IsShowing();
        return false;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate started");
        ConfigValidator.validateConfigFiles(this);
        super.onCreate(savedInstanceState);
        instance = this;
        
        mHeightProvider = new HeightProvider(this);
        mHeightProvider.setHeightListener(this);
        mAttachEdit = new AttachEdit(this);
        mKeyboard = new CustomKeyboard(this);
        
        hud_main = (ConstraintLayout) getLayoutInflater().inflate(R.layout.hud, null);
        addContentView(hud_main, new ConstraintLayout.LayoutParams(-1, -1));
        hud_main.setVisibility(View.GONE);

        loadingscreen = (ConstraintLayout) getLayoutInflater().inflate(R.layout.loading_screen, null);
        addContentView(loadingscreen, new ConstraintLayout.LayoutParams(-1, -1));
        loadingscreen.setVisibility(View.VISIBLE);

        applyDirectConnectExtras(getIntent());
        
        // Memulai inisialisasi native di thread terpisah agar UI tidak hang
        new android.os.Handler(Looper.getMainLooper()).postDelayed(() -> {
            try { 
                Log.d(TAG, "Initializing native SAMP");
                initializeSAMP(); 
                setAllowNextNativePauseMenu(false); 
            } catch (Throwable e) { 
                Log.e(TAG, "Native init failed", e); 
            }
        }, 500);
        
        hideSystemUI();
        Log.d(TAG, "onCreate finished");
    }

    private void applyDirectConnectExtras(Intent intent) {
        if (intent == null) return;
        String nick = intent.getStringExtra(EXTRA_NICKNAME);
        if (nick != null) saveNick(nick);
    }

    private void saveNick(String n) {
        try { Wini wini = new Wini(new File(getExternalFilesDir(null), "SAMP/settings.ini")); wini.put("client", "name", n); wini.store(); } catch (Exception e) {}
    }

    public void hideSystemUI() {
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }
    public void showSystemUI() { getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE); }

    @Override public void onWindowFocusChanged(boolean hasFocus) { super.onWindowFocusChanged(hasFocus); if (hasFocus) hideSystemUI(); }
    public native void togglePlayer(int toggle);
    @Override protected void onNewIntent(Intent intent) { super.onNewIntent(intent); setIntent(intent); applyDirectConnectExtras(intent); }
    @Override public void onResume() { super.onResume(); mHeightProvider.init(getWindow().getDecorView()); }

    public native void onClickButton(int action);
    native void MostrarChat();

    public void showhud() { iShowHud = 1; runOnUiThread(() -> { if (hud_main != null) hud_main.setVisibility(View.VISIBLE); }); }
    public void hidehud() { iShowHud = 0; runOnUiThread(() -> { if (hud_main != null) hud_main.setVisibility(View.GONE); }); }

    @Override public void onHeightChanged(int orientation, int height) { mKeyboard.onHeightChanged(height); }
    @Override public void onBackPressed() { onEventBackPressed(); hideSystemUI(); }
    public native void onEventBackPressed();
}
