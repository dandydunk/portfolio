package com.wingmand;

import android.app.ProgressDialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import com.wingmanlib.WingManWebServiceInbox;
import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceUser;

public class ViewUserProfileActivity extends AppCompatActivity {
    WingManWebServiceInbox inbox = new WingManWebServiceInbox();

    @Override
    protected  void onStart() {
        Log.d("trace", "get user profile onstart");
        super.onStart();
    }

    @Override
    protected  void onResume() {
        Log.d("trace", "get user profile onresume");
        super.onResume();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("trace", "get user profile oncreate");

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_view_user_profile);
        final Toolbar toolbar = (Toolbar)findViewById(R.id.my_toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        final ImageView imageView = (ImageView) findViewById(R.id.profilePic);
        final ViewUserProfileActivity context = this;
        final Button newInboxButton = (Button)findViewById(R.id.buttonNewInbox);
        newInboxButton.setEnabled(false);
        Log.d("info", "id = "+SessionManager.viewUserId);

        final ProgressDialog pd = ProgressDialog.show(this, "Loading...", "Getting the user profile...");
        new Thread(new Runnable() {
            @Override
            public void run() {
                WingManWebServiceUser u = new WingManWebServiceUser();
                u.setUserId(SessionManager.viewUserId);
                try {
                    Log.d("info", "getting user profile");
                    u.GetProfileById();
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            pd.dismiss();
                        }
                    });
                    return;
                }

                final WingManWebServiceInbox inbox = context.inbox;
                inbox.setUserSessionId(SessionManager.getSessionId());
                try {
                    Log.d("info", "getting inbox");
                    inbox.GetInbox(SessionManager.viewUserId);
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            pd.dismiss();
                        }
                    });
                    return;
                }

                final String userName = u.getUserName();
                final Bitmap bPic = u.getBytesProfilePic();
                runOnUiThread(new Runnable() {
                    public void run() {
                        getSupportActionBar().setTitle(userName);
                        imageView.setImageBitmap(bPic);

                        if(inbox.getId() == 0) {
                            ShowNewInboxButton(context);
                        }

                        else {
                            switch(inbox.getStage()) {
                                case 0:
                                    if(inbox.getStartUserId() != SessionManager.getUserId()) {
                                        ShowRejectInboxButton(context);
                                        ShowApproveInboxButton(context);
                                    }
                                    else {
                                        ShowCancelInboxButton(context);
                                    }
                                break;

                                case 1:
                                    ShowSendInboxMessageButton(context);
                                break;
                            }
                        }
                        pd.dismiss();
                    }
                });
            }
        }).start();
    }

    void HideNewInboxButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonNewInbox);
        b.setVisibility(View.GONE);
    }

    void ShowNewInboxButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonNewInbox);
        b.setVisibility(View.VISIBLE);
        b.setEnabled(true);
    }

    void ShowRejectInboxButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonRejectInbox);
        b.setVisibility(View.VISIBLE);
        b.setEnabled(true);
    }

    void HideRejectInboxButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonRejectInbox);
        b.setVisibility(View.GONE);
    }

    void HideApproveInboxButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonApproveInbox);
        b.setVisibility(View.GONE);
    }

    void ShowApproveInboxButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonApproveInbox);
        b.setVisibility(View.VISIBLE);
        b.setEnabled(true);
    }

    void ShowCancelInboxButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonCancelInbox);
        b.setVisibility(View.VISIBLE);
        b.setEnabled(true);
    }

    void HideCancelInboxButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonCancelInbox);
        b.setVisibility(View.GONE);
    }

    void ShowSendInboxMessageButton(ViewUserProfileActivity context) {
        Button b = (Button)context.findViewById(R.id.buttonSendInboxMessage);
        b.setVisibility(View.VISIBLE);
        b.setEnabled(true);
    }

    public void buttonApproveInboxClick(View view) {
        final Button buttonAccept = (Button)view;
        buttonAccept.setEnabled(false);
        final ViewUserProfileActivity context = this;
        new Thread(new Runnable() {
            @Override
            public void run() {
                WingManWebServiceInbox inbox = new WingManWebServiceInbox();
                inbox.setUserSessionId(SessionManager.getSessionId());
                try {
                    Log.d("info", "approving the inbox..");
                    inbox.Approve(context.inbox.getId());
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            buttonAccept.setEnabled(true);
                        }
                    });
                    return;
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        HideApproveInboxButton(context);
                        HideRejectInboxButton(context);
                        ShowSendInboxMessageButton(context);
                    }
                });
            }
        }).start();
    }

    public void buttonRejectInboxClick(View view) {
        final Button buttonAccept = (Button)view;
        buttonAccept.setEnabled(false);
        final ViewUserProfileActivity context = this;
        new Thread(new Runnable() {
            @Override
            public void run() {
                WingManWebServiceInbox inbox = new WingManWebServiceInbox();
                inbox.setUserSessionId(SessionManager.getSessionId());
                try {
                    Log.d("info", "rejecting the inbox..");
                    inbox.Reject(context.inbox.getId());
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            buttonAccept.setEnabled(true);
                        }
                    });
                    return;
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        ShowNewInboxButton(context);
                        HideRejectInboxButton(context);
                        HideApproveInboxButton(context);
                    }
                });
            }
        }).start();
    }

    public void buttonCancelInboxClick(View view) {
        Log.d("info", "canceling the chat invite["+inbox.getId()+"]");

        final Button buttonAccept = (Button)view;
        buttonAccept.setEnabled(false);
        final ViewUserProfileActivity context = this;
        new Thread(new Runnable() {
            @Override
            public void run() {
                WingManWebServiceInbox inbox = new WingManWebServiceInbox();
                inbox.setUserSessionId(SessionManager.getSessionId());
                try {
                    inbox.Cancel(context.inbox.getId());
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            buttonAccept.setEnabled(true);
                        }
                    });
                    return;
                }

                runOnUiThread(new Runnable() {
                    public void run() {
                        HideCancelInboxButton(context);
                        ShowNewInboxButton(context);
                    }
                });
            }
        }).start();
    }

    public void buttonNewInboxClick(View view) {
        final Button newInboxButton = (Button)findViewById(R.id.buttonNewInbox);
        newInboxButton.setEnabled(false);
        final ViewUserProfileActivity context = this;
        new Thread(new Runnable() {
            @Override
            public void run() {
                inbox.setUserSessionId(SessionManager.getSessionId());
                try {
                    Log.d("info", "requesting a new inbox...");
                    inbox.NewInbox(SessionManager.viewUserId);
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(context, ex.getMessage());
                            newInboxButton.setEnabled(true);
                        }
                    });
                    return;
                }

                runOnUiThread(new Runnable() {
                    public void run() {
                        HideNewInboxButton(context);
                        ShowCancelInboxButton(context);
                    }
                });
            }
        }).start();
    }

    public void buttonSendInboxMessageClick(View view) {
        SessionManager.viewInboxId = inbox.getId();
        Log.d("trace", "opening chat for ["+SessionManager.viewInboxId+"]");
        Intent i = new Intent(this, ChatActivity.class);
        startActivity(i);
    }
}
