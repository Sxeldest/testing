package com.xyron.game.launcher;

import android.Manifest;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.downloader.Error;
import com.downloader.OnDownloadListener;
import com.downloader.PRDownloader;
import com.downloader.PRDownloaderConfig;
import com.xyron.game.R;
import com.xyron.game.launcher.data.FilesData;
import com.xyron.game.launcher.util.ConfigValidator;
import com.xyron.game.launcher.util.DataVariantPreferences;
import com.xyron.game.launcher.util.UpdateSourceResolver;
import com.xyron.game.main.SAMP;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class EntryActivity extends SampActivity {

    private final String[] permissions = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.RECORD_AUDIO
    };

    private AlertDialog downloadDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.loading_screen);

        PRDownloader.initialize(getApplicationContext(), PRDownloaderConfig.newBuilder()
                .setDatabaseEnabled(true)
                .build());

        if (isPermissionsGranted()) {
            checkAndLaunch();
        } else {
            ActivityCompat.requestPermissions(this, permissions, 101);
        }
    }

    private boolean isPermissionsGranted() {
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    private void checkAndLaunch() {
        ConfigValidator.validateConfigFiles(this);
        if (isDataMissing()) {
            showDownloadChoiceDialog();
        } else {
            launchGame();
        }
    }

    private boolean isDataMissing() {
        File dataDir = new File(getExternalFilesDir(null), "texdb");
        return !dataDir.exists();
    }

    private void showDownloadChoiceDialog() {
        new AlertDialog.Builder(this)
                .setTitle("Download Data")
                .setMessage("Data game tidak ditemukan. Pilih varian data untuk diunduh:")
                .setPositiveButton("Data Full", (dialog, which) -> startDownloadProcess(DataVariantPreferences.DATA_VARIANT_FULL))
                .setNegativeButton("Data Lite", (dialog, which) -> startDownloadProcess(DataVariantPreferences.DATA_VARIANT_LITE))
                .setCancelable(false)
                .show();
    }

    private void startDownloadProcess(String variant) {
        DataVariantPreferences.saveSelectedVariantId(this, variant);
        
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Mengunduh Data...");
        builder.setView(R.layout.loading_screen);
        builder.setCancelable(false);
        downloadDialog = builder.create();
        downloadDialog.show();

        new Thread(() -> {
            try {
                UpdateSourceResolver.UpdateSourceConfig config = UpdateSourceResolver.resolve(this, variant);
                String nextUrl = config.huggingFaceTreeApiUrl;
                
                if (nextUrl.isEmpty()) {
                    runOnUiThread(() -> Toast.makeText(this, "Konfigurasi download tidak ditemukan", Toast.LENGTH_SHORT).show());
                    return;
                }

                List<JSONObject> fileEntries = new ArrayList<>();
                Set<String> remotePaths = new HashSet<>();

                // Paginasi Hugging Face (Sama seperti newsrp)
                while (nextUrl != null && !nextUrl.isEmpty()) {
                    HttpURLConnection connection = (HttpURLConnection) new URL(nextUrl).openConnection();
                    connection.setConnectTimeout(30000);
                    connection.setReadTimeout(30000);
                    connection.connect();

                    if (connection.getResponseCode() != 200) {
                        throw new IOException("Hugging Face API Error: " + connection.getResponseCode());
                    }

                    BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
                    StringBuilder buffer = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) buffer.append(line);

                    JSONArray entries = new JSONArray(buffer.toString());
                    for (int i = 0; i < entries.length(); i++) {
                        JSONObject item = entries.optJSONObject(i);
                        if (item == null || !"file".equalsIgnoreCase(item.optString("type"))) continue;
                        String path = item.optString("path");
                        if (!path.isEmpty()) {
                            fileEntries.add(item);
                            remotePaths.add(path);
                        }
                    }

                    nextUrl = parseNextLink(connection.getHeaderField("Link"));
                    connection.disconnect();
                }

                List<FilesData> toDownload = new ArrayList<>();
                for (JSONObject entry : fileEntries) {
                    FilesData fileData = buildHuggingFaceFileData(entry, remotePaths, config);
                    if (fileData != null) {
                        // Cek apakah file sudah ada dan ukurannya sama
                        File localFile = new File(getExternalFilesDir(null), fileData.getPath());
                        if (!localFile.exists() || localFile.length() != fileData.getSize()) {
                            toDownload.add(fileData);
                        }
                    }
                }

                if (toDownload.isEmpty()) {
                    runOnUiThread(() -> {
                        if (downloadDialog != null) downloadDialog.dismiss();
                        launchGame();
                    });
                    return;
                }

                downloadNextFile(toDownload, 0);

            } catch (Exception e) {
                e.printStackTrace();
                runOnUiThread(() -> Toast.makeText(this, "Gagal: " + e.getMessage(), Toast.LENGTH_LONG).show());
            }
        }).start();
    }

    private String parseNextLink(String linkHeader) {
        if (linkHeader == null || linkHeader.isEmpty()) return "";
        String[] parts = linkHeader.split(",");
        for (String part : parts) {
            if (part.contains("rel=\"next\"")) {
                int start = part.indexOf('<');
                int end = part.indexOf('>');
                if (start >= 0 && end > start) return part.substring(start + 1, end);
            }
        }
        return "";
    }

    private FilesData buildHuggingFaceFileData(JSONObject entry, Set<String> remotePaths, UpdateSourceResolver.UpdateSourceConfig config) {
        String remotePath = entry.optString("path");
        String prefix = config.huggingFaceFilesPathPrefix;
        String localPath = "";

        if (!prefix.isEmpty()) {
            if (remotePath.equals(prefix)) return null;
            if (!remotePath.startsWith(prefix + "/")) return null;
            localPath = remotePath.substring(prefix.length() + 1);
        } else {
            if (remotePath.startsWith("files/")) {
                String stripped = remotePath.substring(6);
                if (remotePaths.contains(stripped)) return null;
                localPath = stripped;
            } else {
                localPath = remotePath;
            }
        }

        if (localPath.isEmpty() || localPath.startsWith(".")) return null;

        String fileName = new File(localPath).getName();
        long size = entry.optLong("size", 0);
        String downloadUrl = config.huggingFaceResolveBaseUrl + "/" + remotePath;

        return new FilesData(fileName, size, localPath, downloadUrl);
    }

    private void downloadNextFile(List<FilesData> list, int index) {
        if (index >= list.size()) {
            runOnUiThread(() -> {
                if (downloadDialog != null) downloadDialog.dismiss();
                launchGame();
            });
            return;
        }

        FilesData file = list.get(index);
        File localFile = new File(getExternalFilesDir(null), file.getPath());
        localFile.getParentFile().mkdirs();

        PRDownloader.download(file.getUrl(), localFile.getParent(), localFile.getName())
                .build()
                .start(new OnDownloadListener() {
                    @Override
                    public void onDownloadComplete() {
                        downloadNextFile(list, index + 1);
                    }

                    @Override
                    public void onError(Error error) {
                        runOnUiThread(() -> Toast.makeText(EntryActivity.this, "Gagal download " + file.getName(), Toast.LENGTH_SHORT).show());
                    }
                });
    }

    private void launchGame() {
        Intent gameIntent = new Intent(this, SAMP.class);
        if (getIntent() != null && getIntent().getExtras() != null) {
            gameIntent.putExtras(getIntent().getExtras());
        }
        gameIntent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
        startActivity(gameIntent);
        overridePendingTransition(0, 0);
        finish();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 101 && grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            checkAndLaunch();
        } else if (requestCode == 101) {
            Toast.makeText(this, "Izin diperlukan.", Toast.LENGTH_LONG).show();
            finish();
        }
    }
}
