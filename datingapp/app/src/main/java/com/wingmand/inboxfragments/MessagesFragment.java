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
import com.wingmand.ViewUserProfileActivity;
import com.wingmand.WingManService;
import com.wingmanlib.InboxMessagePreview;
import com.wingmanlib.WingManInboxMessage;
import com.wingmanlib.WingManWebServiceUser;

import java.util.ArrayList;

/**
 * A simple {@link Fragment} subclass.
 */
public class MessagesFragment extends Fragment {
    ArrayList<WingManInboxMessage> itemName = new ArrayList<WingManInboxMessage>();
    InboxMessagesListViewArrayAdapter adapter;
    ThreadGetNewMessagesFromDB tgm;
    Thread t;
    ProgressDialog progressDialog;

    public MessagesFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        Log.d("info", "inbox message fragment[createView]");
        View view = inflater.inflate(R.layout.fragment_messages, container, false);

        ListView listView = (ListView)view.findViewById(R.id.listViewInboxMessages);
        adapter = new InboxMessagesListViewArrayAdapter(getActivity(), itemName);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> myAdapter, View myView, int myItemInt, long mylng) {
                Log.d("trace", "clicked chat message["+myItemInt+"]");
                WingManInboxMessage wmim = itemName.get(myItemInt);
                SessionManager.viewInboxId = wmim.inboxId;
                Intent i = new Intent(getActivity(), ChatActivity.class);
                startActivity(i);
            };
        });

        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d("info", "inbox message fragment[onResume]");

        if(null == tgm) {
            tgm = new ThreadGetNewMessagesFromDB();
            tgm.adapter = adapter;
            tgm.context = getActivity();
            tgm.progressDialog = progressDialog;

            tgm.listMessages = itemName;

            t = new Thread(tgm);
            t.start();
        }

        Intent intent = new Intent(getActivity(), WingManService.class);
        getActivity().startService(intent);
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d("info", "inbox message fragment[onPause]");
        if(null != tgm) {
            tgm.stopThread = true;
            try {
                this.t.join();
            } catch(Exception ex){}
            this.tgm = null;
            this.t = null;
        }

        Intent intent = new Intent(getActivity(), WingManService.class);
        getActivity().stopService(intent);
    }

    @Override
    public void onStart() {
        super.onStart();
        Log.d("info", "inbox message fragment[onStart]");

        progressDialog = ProgressDialog.show(getActivity(), "Loading...", "Getting your chats...");
    }

    @Override
    public void onStop() {
        super.onStop();
        Log.d("info", "inbox message fragment[onStop]");
    }
}

class ThreadGetNewMessagesFromDB implements Runnable {
    public boolean stopThread = false;
    ArrayList<WingManInboxMessage> listMessages = null;
    InboxMessagesListViewArrayAdapter adapter = null;
    public Activity context = null;
    public ProgressDialog progressDialog;

    public int CheckMessageExist(int id) {
        for(int i = 0; i < listMessages.size(); i++) {
            if(listMessages.get(i).inboxId == id) {
                return i;
            }
        }

        return -1;
    }

    @Override
    public void run() {
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

        final InboxMessagesListViewArrayAdapter adapterC = adapter;
        int lastMessageIndex = 0;
        while(!stopThread) {
            ArrayList<WingManInboxMessage> listMessagest = null;

            try {
                Log.v("trace", "getting messages from the database...");
                listMessagest = WingManInboxMessage.Get(context, lastMessageIndex, SessionManager.getUserId());
                Log.v("trace", "got message...");
            } catch(Exception ex) {
                Log.d("Info", "exception", ex);
            }

            int lmSize = listMessagest.size();
            if(lmSize > 0) {
                for(int i = 0; i < lmSize; i++) {
                    WingManInboxMessage imp = listMessagest.get(i);
                    if(imp.id > lastMessageIndex) {
                        lastMessageIndex = imp.id;
                    }

                    int mIndex = CheckMessageExist(imp.inboxId);
                    if(mIndex > -1) {
                        //Log.d("trace", "removing message");
                        listMessages.remove(mIndex);
                    }

                    listMessages.add(imp);
                }

                context.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                    //Log.d("trace", "new message was added");
                    adapterC.notifyDataSetChanged();
                    }
                });
            }

            if(progressDialog != null) {
                Log.d("trace", "killing proksjsjsjsj");
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
    }
}
