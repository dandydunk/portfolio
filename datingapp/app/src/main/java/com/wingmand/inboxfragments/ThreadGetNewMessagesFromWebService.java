package com.wingmand.inboxfragments;

import android.app.Activity;
import android.content.Context;
import android.util.Log;

import com.wingmand.SessionManager;
import com.wingmanlib.InboxMessagePreview;
import com.wingmanlib.WingManInboxMessage;
import com.wingmanlib.WingManWebServiceInbox;

import java.util.ArrayList;

/**
 * Created by deymack on 3/31/2017.
 */

public class ThreadGetNewMessagesFromWebService implements Runnable {
    public boolean stopThread = false;
    public Context context = null;

    @Override
    public void run() {
        WingManWebServiceInbox mInbox = new WingManWebServiceInbox();
        mInbox.setUserSessionId(SessionManager.getSessionId());
        while(!stopThread) {
            ArrayList<WingManInboxMessage> listMessagest = null;

            try {
                listMessagest = mInbox.GetNewMessages();
            } catch(Exception ex) {
                Log.d("Info", "exception", ex);

                break;
            }

            int lmSize = listMessagest.size();
            if(lmSize > 0) {
                for(int i = 0; i < lmSize; i++) {
                    WingManInboxMessage wInboxMessage = listMessagest.get(i);
                    wInboxMessage.toUserId = SessionManager.getUserId();
                    wInboxMessage.Save(this.context);
                }
            }
        }
    }
}
