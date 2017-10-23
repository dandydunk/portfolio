package com.wingmand;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import com.wingmanlib.WingManInboxMessage;
import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceInbox;
import com.wingmanlib.WingManWebServiceUser;

import java.util.ArrayList;
import java.util.Stack;

public class ChatActivity extends AppCompatActivity {
    ThreadGetNewMessagesFromDBForChat tcb = null;
    Thread t;
    Thread thread2;

    public Stack<ChatMessage> stackChatMessages = new Stack<ChatMessage>();
    Object lock1 = new Object();
    Object waiter = new Object();
    ProgressDialog progressDialog;
    ThreadSendMessages threadSendMessages;
    WingManWebServiceUser otherUser = null;

    class ChatMessage {
        public String message;
        public int inboxId;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("trace", "chat activity on create called");

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chat);

        Toolbar myToolbar = (Toolbar) findViewById(R.id.my_toolbar);
        setSupportActionBar(myToolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        final EditText et = (EditText)findViewById(R.id.editTextChatMessage);
        et.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_NULL && event.getAction() == KeyEvent.ACTION_DOWN) {
                    ChatMessage cm = new ChatMessage();
                    cm.message = et.getText().toString().trim();
                    if(cm.message.length() == 0) {
                        return true;
                    }
                    cm.inboxId = SessionManager.viewInboxId;

                    synchronized (lock1) {
                        stackChatMessages.push(cm);
                    }
                    et.setText("");
                }
                return true;

            }
        });
    }

    public void getOtherUser() {
        //final ProgressDialog pd = ProgressDialog.show(this, "Loading...", "Please wait...");
        final Activity context = this;

        WingManWebServiceInbox inbox = new WingManWebServiceInbox();
        inbox.setUserSessionId(SessionManager.getSessionId());
        inbox.setId(SessionManager.viewInboxId);
        int oUserId = 0;
        try {
            Log.d("info", "getting inbox");
            oUserId = inbox.GetInboxOtherUser();
        } catch(final Exception ex) {
            Log.d("stackTrace", "exception", ex);
            runOnUiThread(new Runnable() {
                public void run() {
                    WingManUtils.ShowErrorDialog(context, ex.getMessage());
                    //pd.dismiss();
                }
            });
            return;
        }

        final WingManWebServiceUser u = new WingManWebServiceUser();
        u.setUserId(oUserId);
        try {
            u.GetProfileById();
        } catch(final Exception ex) {
            Log.d("stackTrace", "exception", ex);
            runOnUiThread(new Runnable() {
                public void run() {
                    WingManUtils.ShowErrorDialog(context, ex.getMessage());
                    //pd.dismiss();
                }
            });
            return;
        }

        otherUser = u;
        runOnUiThread(new Runnable() {
            public void run() {
                getSupportActionBar().setTitle(u.getUserName());
                //pd.dismiss();
            }
        });
    }

    @Override
    public void onStart(){
        Log.d("trace", "chat activity on start called");
        super.onStart();
        LinearLayout ll = (LinearLayout)findViewById(R.id.linearLayoutChatMessages);
        ll.removeAllViews();

        progressDialog = null;//ProgressDialog.show(this, "Loading...", "Getting chat history...");
    }

    @Override
    public void onPause(){
        Log.d("trace", "chat activity on pause called");
        super.onPause();

        if(tcb != null) {
            tcb.stopThread = true;
            try {
                this.t.join();
            } catch (Exception ex) {
            }

            tcb = null;
            t = null;
        }

        if(thread2 != null) {
            threadSendMessages.stop = true;

            try {
                thread2.join();
            }catch(Exception ex) {}
        }

        Intent intent = new Intent(this, WingManService.class);
        stopService(intent);

        Log.d("trace", "chat activity on pause exited");
    }

    @Override
    public void onResume(){
        Log.d("trace", "chat activity on resume called");
        super.onResume();

        if(null == tcb) {
            tcb = new ThreadGetNewMessagesFromDBForChat();
            tcb.chatActivity = this;
            tcb.context = this;
            tcb.progressDialog = progressDialog;

            t = new Thread(tcb);
            t.start();
        }

        if(null == thread2) {
            threadSendMessages = new ThreadSendMessages();
            threadSendMessages.lock1 = lock1;
            threadSendMessages.stackChatMessages = stackChatMessages;

            thread2 = new Thread(threadSendMessages);
            thread2.start();
        }

        Intent intent = new Intent(this, WingManService.class);
        startService(intent);
    }

    @Override
    public void onStop(){
        Log.d("trace", "chat activity on stop called");
        super.onStop();
    }

    @Override
    public void onDestroy(){
        Log.d("trace", "chat activity on destroy called");
        super.onDestroy();


    }

    public void AddChatMessage(Activity context, WingManInboxMessage message) {
        LinearLayout ll = (LinearLayout)context.findViewById(R.id.linearLayoutChatMessages);
        LayoutInflater inflater = LayoutInflater.from(context);
        View view = inflater.inflate(R.layout.chat_layout, null);

        TextView textViewMessage = (TextView)view.findViewById(R.id.textViewMessage);
        textViewMessage.setText(message.message);

        ImageView ivIcon = (ImageView)view.findViewById(R.id.icon);
        if(message.fromUserId != SessionManager.getUserId()) {
            ivIcon.setImageBitmap(otherUser.getBytesProfilePic());
            textViewMessage.setTextColor(Color.parseColor("#464646"));
        }
        else {
            ivIcon.setVisibility(View.GONE);
            textViewMessage.setTextColor(Color.parseColor("#7F7E7E"));
        }

        ll.addView(view);

        final ScrollView sv = (ScrollView)context.findViewById(R.id.scrollViewChat);
        sv.post(new Runnable() {
            @Override
            public void run() {
                sv.fullScroll(ScrollView.FOCUS_DOWN);
            }
        });
    }
}

class ThreadSendMessages implements  Runnable {
    public boolean stop = false;
    public Stack<ChatActivity.ChatMessage> stackChatMessages = new Stack<ChatActivity.ChatMessage>();
    public Object lock1;

    @Override
    public void run() {
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

        while(!stop) {
            //Log.d("trace", "waiting for messages to send...");
            while(stackChatMessages.empty()) {
                if(stop) {
                    break;
                }
                try {
                    Thread.sleep(1);
                } catch (Exception ex) {
                }
            }

            if(stop) {
                break;
            }

            //Log.d("trace", "sending messages...");
            ChatActivity.ChatMessage chatMsg = null;
            synchronized (lock1) {
                chatMsg = stackChatMessages.pop();
            }

            WingManWebServiceInbox i = new WingManWebServiceInbox();
            i.setUserSessionId(SessionManager.getSessionId());
            i.setId(chatMsg.inboxId);
            try {
                i.NewMessage(chatMsg.message);
            } catch(Exception ex) {
                Log.d("trace", "exception", ex);
                continue;
            }
        }
    }
}

class ThreadGetNewMessagesFromDBForChat implements Runnable {
    public boolean stopThread = false;
    public Activity context = null;
    public ChatActivity chatActivity = null;
    public int inboxId = 0;
    boolean threadGoing = false;
    public ProgressDialog progressDialog;

    @Override
    public void run() {
        //Looper.prepare();

        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

        threadGoing = true;
        int lastMessageIndex = 0;

        if(null == chatActivity.otherUser) {
            chatActivity.getOtherUser();
        }

        while(!stopThread) {
            ArrayList<WingManInboxMessage> listMessagest = null;

            try {
                //Log.v("trace", "getting messages["+SessionManager.viewInboxId+"] from the database for chat...");
                listMessagest = WingManInboxMessage.GetByInboxId(context, SessionManager.viewInboxId, lastMessageIndex);
            } catch(Exception ex) {
                Log.d("Info", "exception", ex);
            }

            int lmSize = listMessagest.size();
            if(lmSize > 0) {
                for(int i = 0; i < lmSize; i++) {
                    final WingManInboxMessage imp = listMessagest.get(i);
                    if(imp.id > lastMessageIndex) {
                        lastMessageIndex = imp.id;
                    }

                    context.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            chatActivity.AddChatMessage(context, imp);
                        }
                    });
                }
            }

            if(progressDialog != null) {
                context.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if(progressDialog != null) {
                            progressDialog.dismiss();
                            progressDialog = null;
                        }

                    }
                });
            }
        }

        threadGoing = false;
        Log.d("trace", "thread stopped getting chats");
    }
}
