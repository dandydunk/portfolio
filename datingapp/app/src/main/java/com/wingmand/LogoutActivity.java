package com.wingmand;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.wingmanlib.WingManSettings;

public class LogoutActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_logout);

        WingManSettings settings = new WingManSettings(this);
        settings.UpdateSetting("sessionId", "");
        settings.UpdateSetting("userId", "");


        SessionManager.setSessionId(null);
        SessionManager.setUserId(0);

        Intent intent = new Intent(this, MainActivity.class);
        startActivity(intent);
    }
}
