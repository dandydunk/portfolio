package com.wingmand;

import android.app.ProgressDialog;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceUser;

import java.io.ByteArrayOutputStream;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;

public class UploadPhotosActivity extends AppCompatActivity {
    Uri uploadPhotoPath = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_upload_photos);
        Toolbar toolbar = (Toolbar)findViewById(R.id.my_toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
    }

    public void buttonNewPhotoClick(View view) {
        Log.d("info", "button new photo clicked");
        Intent photoPickerIntent = new Intent(Intent.ACTION_GET_CONTENT);
        photoPickerIntent.setType("image/*");
        startActivityForResult(photoPickerIntent, 1);
    }

    public void buttonUploadPhotoClick(View view) {
        final Button thisButton = (Button)view;
        thisButton.setEnabled(false);

        final ImageView picToUpload = (ImageView)findViewById(R.id.imageViewPicToUpload);
        final UploadPhotosActivity context = this;
        final ProgressDialog pd = ProgressDialog.show(this, "Loading...", "Uploading...");
        new Thread(new Runnable() {
            @Override
            public void run() {
                android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

                InputStream inputStream = null;
                try {
                    inputStream = getContentResolver().openInputStream(uploadPhotoPath);
                } catch(final IOException ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            pd.dismiss();
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            thisButton.setEnabled(true);
                        }
                    });
                    return;
                }

                ByteArrayOutputStream output = new ByteArrayOutputStream();
                try {
                    int bytesRead;
                    byte[] buffer = new byte[8192];
                    while ((bytesRead = inputStream.read(buffer)) != -1) {
                        output.write(buffer, 0, bytesRead);
                    }
                } catch (final IOException ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            pd.dismiss();
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            thisButton.setEnabled(true);
                        }
                    });
                    return;
                }

                String encoded = null;
                try {
                    encoded = URLEncoder.encode(Base64.encodeToString(output.toByteArray(), Base64.DEFAULT), "utf-8");
                } catch(final UnsupportedEncodingException ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            pd.dismiss();
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            thisButton.setEnabled(true);
                        }
                    });
                    return;
                }

                Log.d("info", "uploading ["+encoded+"]...");
                WingManWebServiceUser user = new WingManWebServiceUser();
                user.setSessionId(SessionManager.getSessionId());
                try {
                    user.UploadPhoto(encoded);
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            pd.dismiss();
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            thisButton.setEnabled(true);
                        }
                    });
                    return;
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Intent intent = new Intent(context, ManagePhotosActivity.class);
                        startActivity(intent);
                        pd.dismiss();
                    }
                });


            }
        }).start();
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Uri selectedImage = data.getData();
        String[] filePathColumn = {MediaStore.Images.Media.DATA};

        uploadPhotoPath = selectedImage;

        Uri uri = data.getData();
        Log.d("info", "image url = ["+uri.toString()+"]");
        ImageView picToUpload = (ImageView)findViewById(R.id.imageViewPicToUpload);
        try {
            picToUpload.setImageBitmap(getBitmapFromUri(selectedImage));
        } catch(IOException ex) {
            Log.d("error", ex.getMessage());
        }

        picToUpload.getDrawable();
    }

    private Bitmap getBitmapFromUri(Uri uri) throws IOException {
        ParcelFileDescriptor parcelFileDescriptor =
                getContentResolver().openFileDescriptor(uri, "r");
        FileDescriptor fileDescriptor = parcelFileDescriptor.getFileDescriptor();

        Bitmap image = BitmapFactory.decodeFileDescriptor(fileDescriptor);
        parcelFileDescriptor.close();
        return image;
    }
}
