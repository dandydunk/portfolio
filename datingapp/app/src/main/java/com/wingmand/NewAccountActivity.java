package com.wingmand;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.widget.EditText;
import android.view.View;
import android.widget.Button;
import android.os.Handler;
import android.os.Looper;

import com.facebook.AccessToken;
import com.facebook.CallbackManager;
import com.facebook.FacebookCallback;
import com.facebook.FacebookException;
import com.facebook.login.LoginResult;
import com.facebook.login.widget.LoginButton;
import com.wingmanlib.WingManSettings;
import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceUser;

public class NewAccountActivity extends AppCompatActivity {
    CallbackManager callbackManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_new_account);
        Toolbar toolbar = (Toolbar)findViewById(R.id.my_toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        LoginButton loginButton = (LoginButton)findViewById(R.id.buttonFacebookLogin);
        loginButton.setReadPermissions("email");
        final Activity context = this;
        callbackManager = CallbackManager.Factory.create();
        loginButton.registerCallback(callbackManager, new FacebookCallback<LoginResult>() {
            @Override
            public void onSuccess(LoginResult loginResult) {
                final ProgressDialog pd = ProgressDialog.show(context, "Loading...", "Creating your account...");

                AccessToken at = loginResult.getAccessToken();
                final String token = at.getToken();
                Log.d("trace", "facebook login success ["+token+"]");
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

                        WingManWebServiceUser user = new WingManWebServiceUser();
                        try {
                            Log.d("info", "logging in...");
                            user.FBNewAccount(token);
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

                        Log.d("info", String.format("updating user's session [%s]", user.getSessionId()));
                        WingManSettings settings = new WingManSettings(context);
                        settings.UpdateSetting("sessionId", user.getSessionId());
                        settings.UpdateSetting("userId", Integer.toString(user.getUserId()));

                        SessionManager.setSessionId(user.getSessionId());
                        SessionManager.setUserId(user.getUserId());

                        Log.d("info", String.format("updated user's session [%s]", settings.GetByName("sessionId")));

                        Intent intent = new Intent(context, MainActivity.class);
                        startActivity(intent);
                    }
                }).start();
            }

            @Override
            public void onCancel() {
                Log.d("trace", "facebook login cancel");
            }

            @Override
            public void onError(FacebookException ex) {
                Log.d("trace", "facebook login error", ex);
                WingManUtils.ShowErrorDialog(context, ex.getMessage());
            }
        });
    }

    public void buttonSignUpOnClick(View view) throws Exception {
        Log.d("trace", "buttonSignupOnClick");
        final ProgressDialog pd = ProgressDialog.show(this, "Loading...", "Creating your account...");
        final NewAccountActivity naa = this;
        final Button button = (Button)findViewById(R.id.buttonSignUp);
        button.setEnabled(false);
        final EditText email = (EditText)findViewById(R.id.editTextEmail);
        final EditText password = (EditText)findViewById(R.id.editTextPassword);
        final EditText passwordConfirm = (EditText)findViewById(R.id.editTextConfirmPassword);

        final String emailText = email.getText().toString();
        final String passwordText = password.getText().toString();
        final String passwordConfirmText = passwordConfirm.getText().toString();

        Thread thread = new Thread(new Runnable(){
            @Override
            public void run(){
                android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

                WingManWebServiceUser user = new WingManWebServiceUser();
                user.setEmail(emailText);
                user.setPassword(passwordText);
                user.setPasswordConfirm(passwordConfirmText);
                try {
                    Log.d("info", "making new account...");
                    user.NewAccount();
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

                Log.d("info", String.format("updating user's session [%s]", user.getSessionId()));
                WingManSettings settings = new WingManSettings(naa);
                settings.UpdateSetting("sessionId", user.getSessionId());
                settings.UpdateSetting("userId", Integer.toString(user.getUserId()));
                SessionManager.setSessionId(user.getSessionId());
                SessionManager.setUserId(user.getUserId());
                Log.d("info", String.format("updated user's session [%s]", settings.GetByName("sessionId")));

                Intent intent = new Intent(naa, MainActivity.class);
                startActivity(intent);
            }
        });
        thread.start();
    }
}
