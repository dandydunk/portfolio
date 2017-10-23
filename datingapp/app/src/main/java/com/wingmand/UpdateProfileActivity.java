package com.wingmand;

import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;

import com.wingmanlib.WingManSettings;
import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceUser;
import android.view.Window;

public class UpdateProfileActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.activity_update_profile);
        setProgressBarIndeterminateVisibility(true);
        Toolbar toolbar = (Toolbar)findViewById(R.id.my_toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        final ProgressDialog pd = ProgressDialog.show(this, "Loading...", "Getting your profile...");
        final Button button = (Button)findViewById(R.id.buttonUpdateProfile);
        button.setEnabled(false);

        final EditText editTextUserName = (EditText)findViewById(R.id.editTextUserName);
        final EditText editTextBirthday = (EditText)findViewById(R.id.editTextBirthday);


        final UpdateProfileActivity naa = this;
        new Thread(new Runnable() {
            @Override
            public void run() {
                final WingManWebServiceUser user = new WingManWebServiceUser();
                user.setSessionId(SessionManager.getSessionId());
                try {
                    Log.d("info", "getting the users profile");
                    user.GetProfile();
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            pd.dismiss();
                            WingManUtils.ShowErrorDialog(naa, ex.getMessage());
                            setProgressBarIndeterminateVisibility(false);

                        }
                    });
                    return;
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        editTextUserName.setText(user.getUserName());
                        editTextBirthday.setText(user.getBirthday());
                        button.setEnabled(true);
                        setProgressBarIndeterminateVisibility(false);
                        pd.dismiss();
                    }
                });
            }
        }).start();
    }

    public void buttonUpdateProfileClick(View view) throws Exception {
        Log.d("trace", "buttonUpdateProfileClick");

        final ProgressDialog pd = ProgressDialog.show(this, "Loading...", "Updating your profile...");
        final Button button = (Button)findViewById(R.id.buttonUpdateProfile);
        button.setEnabled(false);

        final EditText editTextUserName = (EditText)findViewById(R.id.editTextUserName);
        final EditText editTextBirthday = (EditText)findViewById(R.id.editTextBirthday);

        final String userNameText = editTextUserName.getText().toString();
        final String birthdayText = editTextBirthday.getText().toString();
        final UpdateProfileActivity naa = this;
        Thread thread = new Thread(new Runnable(){
            @Override
            public void run(){
                Looper.prepare();

                WingManWebServiceUser user = new WingManWebServiceUser();
                user.setUserName(userNameText);
                user.setBirthday(birthdayText);
                user.setSessionId(SessionManager.getSessionId());
                try {
                    Log.d("info", "updating the profile...");
                    user.UpdateProfile();
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            pd.dismiss();
                            WingManUtils.ShowErrorDialog(naa, ex.getMessage());
                            button.setEnabled(true);
                        }
                    });
                    return;
                }
                runOnUiThread(new Runnable() {
                    public void run() {
                        button.setEnabled(true);
                        pd.dismiss();
                    }
                });
            }
        });
        thread.start();
    }
}
