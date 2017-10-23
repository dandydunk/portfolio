package com.wingmand.inboxfragments;


import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ListView;

import com.wingmand.ChatActivity;
import com.wingmand.R;
import com.wingmand.SessionManager;
import com.wingmanlib.WingManInbox;
import com.wingmanlib.WingManInboxMessage;
import com.wingmanlib.WingManWebServiceInbox;

import java.util.ArrayList;


class ThreadGetRequests implements Runnable {
    public boolean stop = false;
    public ArrayList<WingManInbox> listRequests = null;
    public ArrayList<Integer> listInboxIds = new ArrayList<Integer>();
    public Activity context = null;
    public InboxRequestsListViewArrayAdapter adapter = null;
    public ProgressDialog progressDialog;

    @Override
    public void run() {
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

        WingManWebServiceInbox wInbox = new WingManWebServiceInbox();
        wInbox.setUserSessionId(SessionManager.getSessionId());
        do {
            ArrayList<WingManInbox> listRequests_ = null;

            try {
                listRequests_ = wInbox.GetRequests();
            } catch(Exception ex) {
                Log.d("trace", "exception", ex);
            }

            for(int i = 0; i < listRequests_.size(); i++) {
                WingManInbox wmi = listRequests_.get(i);
                if(listInboxIds.indexOf(wmi.id) == -1) {
                    listRequests.add(wmi);
                    listInboxIds.add(wmi.id);
                }
            }

            context.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                adapter.notifyDataSetChanged();
                if(progressDialog != null) {
                    progressDialog.dismiss();
                    progressDialog = null;
                }
                }
            });
        } while(!stop);
    }
}

public class RequestsFragment extends Fragment {
    InboxRequestsListViewArrayAdapter adapter = null;
    ArrayList<WingManInbox> itemName = new ArrayList<WingManInbox>();
    Thread thread1 = null;
    ThreadGetRequests threadGetRequests = null;
    ProgressDialog progressDialog;

    public RequestsFragment() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.d("info", "inbox requests fragment[createView]");
        View view = inflater.inflate(R.layout.fragment_requests, container, false);

        ListView listView = (ListView)view.findViewById(R.id.listViewRequests);
        adapter = new InboxRequestsListViewArrayAdapter(getActivity(), itemName);
        listView.setAdapter(adapter);

        return view;
    }

    @Override
    public void onStart() {
        super.onStart();
        Log.d("info", "inbox requests fragment[onStart]");
        progressDialog = ProgressDialog.show(getActivity(), "Loading...", "Getting your requests...");
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d("info", "inbox requests fragment[onResume]");

        if(thread1 == null) {
            threadGetRequests = new ThreadGetRequests();
            threadGetRequests.adapter = adapter;
            threadGetRequests.context = getActivity();
            threadGetRequests.listRequests = itemName;
            threadGetRequests.progressDialog = progressDialog;
            thread1 = new Thread(threadGetRequests);
            thread1.start();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d("info", "inbox requests fragment[onPause]");
        if(thread1 != null) {
            threadGetRequests.stop = true;

            try {
                thread1.join();
            } catch(Exception ex){}

            thread1 = null;
        }
    }

    @Override
    public void onStop() {
        super.onStop();
        Log.d("info", "inbox requests fragment[onStop]");
    }
}
