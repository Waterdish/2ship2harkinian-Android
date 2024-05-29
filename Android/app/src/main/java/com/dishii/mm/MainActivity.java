
package com.dishii.mm;
import org.libsdl.app.SDLActivity;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import android.Manifest;
import android.content.pm.PackageManager;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.util.Log;

//This class is the main SDLActivity and just sets up a bunch of default files
public class MainActivity extends SDLActivity{

    private static final int STORAGE_PERMISSION_REQUEST_CODE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Check if storage permissions are granted
        if (hasStoragePermission()) {
            doVersionCheck();
            setupFiles();
        } else {
            requestStoragePermission();
        }
    }

    private void doVersionCheck(){
        SharedPreferences preferences = getSharedPreferences("com.dishii.mm.prefs",Context.MODE_PRIVATE);
        int currentVersion = BuildConfig.VERSION_CODE; // Use your app's version code
        int storedVersion = preferences.getInt("appVersion", 1);

        if (currentVersion > storedVersion) {
            deleteOutdatedAssets();
            preferences.edit().putInt("appVersion", currentVersion).apply();
        }
    }

    private void deleteOutdatedAssets(){
        File externalSohFile = new File(getExternalFilesDir(null), "2ship.o2r");
        externalSohFile.delete();
        File externalOotFile = new File(getExternalFilesDir(null), "mm.o2r");
        externalOotFile.delete();
        File externalAssetsFolder = new File(getExternalFilesDir(null), "assets");
        deleteRecursive(externalAssetsFolder);

    }

    private void deleteRecursive(File fileOrDirectory) {
        if (fileOrDirectory.isDirectory()) {
            for (File child : fileOrDirectory.listFiles()) {
                deleteRecursive(child);
            }
        }
        fileOrDirectory.delete();
    }



    // Check if storage permission is granted
    private boolean hasStoragePermission() {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED &&
                ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                        == PackageManager.PERMISSION_GRANTED;
    }

    // Request storage permission
    private void requestStoragePermission() {
        ActivityCompat.requestPermissions(this,
                new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE},
                STORAGE_PERMISSION_REQUEST_CODE);
    }

    // Handle permission request result
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == STORAGE_PERMISSION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                setupFiles();
            } else {
                // Permission denied, handle accordingly (e.g., show a message)
            }
        }
    }

    private void setupFiles(){
        //Copy assets folder for rom extraction
        File externalAssetsDir = new File(getExternalFilesDir(null), "assets");
        if (!externalAssetsDir.exists()) {
            try {
                externalAssetsDir.mkdirs();
                AssetCopyUtil.copyAssetsToExternal(this, "assets", externalAssetsDir.getAbsolutePath());
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        //Create empty mods folder
        File externalModsDir = new File(getExternalFilesDir(null), "mods");
        externalModsDir.mkdirs();

        //Copy 2ship.o2r
        File externalSohOtrFile = new File(getExternalFilesDir(null), "2ship.o2r");
        if (!externalSohOtrFile.exists()) {
            try {
                InputStream in = getAssets().open("2ship.o2r");
                OutputStream out = new FileOutputStream(externalSohOtrFile);

                byte[] buffer = new byte[1024];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }

                in.close();
                out.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

    }

    private native void nativeHandleSelectedFile(String filePath);

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 0 && resultCode == RESULT_OK) {
            Uri selectedFileUri = data.getData();
            String fileName = "MM.z64";
            File destinationDirectory = getExternalFilesDir(null); // The second argument can specify a subdirectory, or you can pass null to use the root directory.
            File destinationFile = new File(destinationDirectory, fileName);

            if (destinationDirectory != null) {
                try {
                    InputStream in = getContentResolver().openInputStream(selectedFileUri);
                    OutputStream out = new FileOutputStream(destinationFile);

                    byte[] buffer = new byte[4096];
                    int bytesRead;
                    while ((bytesRead = in.read(buffer)) != -1) {
                        out.write(buffer, 0, bytesRead);
                    }

                    in.close();
                    out.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            nativeHandleSelectedFile(destinationFile.getPath());
        }
    }

    public void openFilePicker() {
        // Create an Intent to open the file picker dialog
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.setType("*/*");

        // Start the file picker dialog
        startActivityForResult(intent, 0);
    }

    // Check if external storage is available and writable
    private boolean isExternalStorageWritable() {
        String state = Environment.getExternalStorageState();
        return Environment.MEDIA_MOUNTED.equals(state);
    }

}
