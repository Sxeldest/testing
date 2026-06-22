package com.xyron.game.launcher;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.xyron.game.R;
import com.xyron.game.launcher.util.ConfigValidator;
import com.xyron.game.main.SAMP;

public class EntryActivity extends SampActivity {

    private final String[] permissions = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.RECORD_AUDIO
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.loading_screen);

        if (isPermissionsGranted()) {
            launchGame();
        } else {
            ActivityCompat.requestPermissions(this, permissions, 101);
        }
    }

    private boolean isPermissionsGranted() {
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(this, permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    private void launchGame() {
        // Menyiapkan file konfigurasi (menyalin settings.ini dari assets jika tidak ada)
        ConfigValidator.validateConfigFiles(this);

        // Langsung menjalankan game (SAMP Activity) tanpa UI launcher
        Intent gameIntent = new Intent(this, SAMP.class);
        if (getIntent() != null && getIntent().getExtras() != null) {
            gameIntent.putExtras(getIntent().getExtras());
        }
        
        // Memastikan tidak ada animasi transisi agar terlihat instan
        gameIntent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
        startActivity(gameIntent);
        overridePendingTransition(0, 0);
        
        // Tutup EntryActivity
        finish();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 101) {
            boolean granted = grantResults.length > 0;
            for (int res : grantResults) {
                granted &= (res == PackageManager.PERMISSION_GRANTED);
            }

            if (granted) {
                launchGame();
            } else {
                Toast.makeText(this, "Izin diperlukan untuk menjalankan game.", Toast.LENGTH_LONG).show();
                finish();
            }
        }
    }
}
