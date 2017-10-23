package com.wingmand;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.wingmand.inboxfragments.ThreadGetNewMessagesFromWebService;

public class WingManService extends Service {
    ThreadGetNewMessagesFromWebService tgw = null;
    Thread t = null;

    public WingManService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }

    @Override
    public void onCreate() {
        Log.d("trace", "wing man service oncreate called");
        if(null == tgw) {
            tgw = new ThreadGetNewMessagesFromWebService();
            tgw.context = this;
            t = new Thread(tgw);
            t.start();
        }
    }

    @Override
    public void onDestroy() {
        Log.d("trace", "wing man service ondestroy called");
        if(null != tgw) {
            tgw.stopThread = true;
            try {
                this.t.join();
            } catch(Exception ex){}
            this.tgw = null;
            this.t = null;
        }
    }
}
