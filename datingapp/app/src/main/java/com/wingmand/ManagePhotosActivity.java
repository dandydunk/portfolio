package com.wingmand;

import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.GridView;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.RelativeLayout;

import com.wingmanlib.WingManPhoto;
import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceUser;

import java.util.ArrayList;

public class ManagePhotosActivity extends AppCompatActivity {
    ArrayList<WingManPhoto> listPhotos = new ArrayList<WingManPhoto>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_manage_photos);
        Toolbar toolbar = (Toolbar)findViewById(R.id.my_toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        final PhotoViewAdapter pva = new PhotoViewAdapter(this, listPhotos);
        final GridView gridView = (GridView)findViewById(R.id.gridViewPhotos);
        gridView.setAdapter(pva);
        final ManagePhotosActivity context = this;
        gridView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, final View view, final int position, long id) {
                final WingManImageView imgView = (WingManImageView)view;
                Log.d("info", "clicked item ["+imgView.getWingManPhoto().getId()+"]");
                PopupMenu pMenu = new PopupMenu(context, view);
                pMenu.getMenuInflater().inflate(R.menu.manage_photo_popup_menu, pMenu.getMenu());
                pMenu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        if(item.getItemId() == R.id.managePhotoMenuItemDelete) {
                            Log.d("info", "deleting item ["+imgView.getWingManPhoto().getId()+"]");
                            new Thread(new Runnable() {
                                @Override
                                public void run() {
                                    Looper.prepare();
                                    WingManWebServiceUser wu = new WingManWebServiceUser();
                                    wu.setSessionId(SessionManager.getSessionId());
                                    try {
                                        wu.DeletePhoto(imgView.getWingManPhoto().getId());
                                    } catch(final Exception ex) {
                                        Log.d("stackTrace", "exception", ex);
                                        runOnUiThread(new Runnable() {
                                            public void run() {
                                                WingManUtils.ShowErrorDialog(context, ex.getMessage());
                                            }
                                        });
                                        return;
                                    }

                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            listPhotos.remove(position);
                                            pva.notifyDataSetChanged();
                                        }
                                    });
                                }
                            }).start();
                        }

                        else if(item.getItemId() == R.id.managePhotoMenuItemMakeMain) {
                            Log.d("info", "setting item default["+imgView.getWingManPhoto().getId()+"]");
                        }
                        return true;
                    }
                });
                pMenu.show();
            }
        });

        final ProgressDialog pd = ProgressDialog.show(this, "Loading...", "Getting your photos...");
        new Thread(new Runnable() {
            @Override
            public void run() {
                android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

                WingManWebServiceUser wu = new WingManWebServiceUser();
                wu.setSessionId(SessionManager.getSessionId());
                ArrayList<WingManPhoto> listPhotos = null;
                try {
                    listPhotos = wu.GetPhotos();
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            pd.dismiss();
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                        }
                    });
                    return;
                }

                int countPhotos = listPhotos.size();

                for(int i = 0; i < countPhotos; i++) {
                    final WingManPhoto wmp = listPhotos.get(i);
                    Log.d("info", "loading photo id["+wmp.getId()+"]");

                    context.listPhotos.add(wmp);
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        pva.notifyDataSetChanged();
                        pd.dismiss();
                    }
                });

            }
        }).start();
    }

    public void buttonUploadNewPhotoClick(View view) {
        Button buttonUpload = (Button)findViewById(R.id.buttonUploadNewPhoto);
        buttonUpload.setEnabled(false);
        Intent intent = new Intent(this, UploadPhotosActivity.class);
        startActivity(intent);
    }
}
