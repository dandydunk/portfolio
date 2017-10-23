package com.wingmand.mainactivityfragments;


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

import com.wingmand.R;
import com.wingmand.SessionManager;
import com.wingmand.ViewUserProfileActivity;
import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceUser;

import java.util.ArrayList;


class ThreadGetUsers implements Runnable {
    public Activity context = null;
    public ArrayList<WingManWebServiceUser> listUsers = new ArrayList<WingManWebServiceUser>();
    public WListViewArrayAdapter adapter;
    public boolean stop = false;
    public ProgressDialog progressDialog = null;

    public int CheckExist(int id) {
        for(int i = 0; i < listUsers.size(); i++) {
            if(listUsers.get(i).getUserId() == id) {
                return i;
            }
        }

        return -1;
    }

    @Override
    public void run() {
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);

        if(null == SessionManager.getSessionId()) {
            return;
        }

        WingManWebServiceUser wu = new WingManWebServiceUser();
        wu.setSessionId(SessionManager.getSessionId());
        ArrayList<WingManWebServiceUser> listUsers = null;
        WingManWebServiceUser wsu;
        while(!stop) {
            try {
                listUsers = wu.GetUsersByRadius();
            } catch (final Exception ex) {
                Log.d("stackTrace", "exception", ex);
                context.runOnUiThread(new Runnable() {
                    public void run() {
                        WingManUtils.ShowErrorDialog(context, ex.getMessage());
                    }
                });
                return;
            }

            //Log.d("info", "got ["+listUsers.size()+"] users");
            int lus = listUsers.size();
            boolean didChange = false;
            for (int i = 0; i < lus; i++) {
                wsu = listUsers.get(i);
                if(CheckExist(wsu.getUserId()) > -1) {
                    continue;
                }

                this.listUsers.add(wsu);
                if(!didChange) {
                    didChange = true;
                }
            }

            if(didChange) {
                context.runOnUiThread(new Runnable() {
                    public void run() {
                    adapter.notifyDataSetChanged();
                    }
                });
            }

            if(progressDialog != null) {
                context.runOnUiThread(new Runnable() {
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

public class NearYouNowFragment extends Fragment {
    ArrayList<WingManWebServiceUser> listUsers = new ArrayList<WingManWebServiceUser>();
    WListViewArrayAdapter adapter;
    ThreadGetUsers threadGetUsers;
    Thread thread1;
    ProgressDialog progressDialog;
    ProgressDialog progressDialog2;

    public ArrayList<Integer> listUsers_ = new ArrayList<Integer>();

    public NearYouNowFragment() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.d("trace", "near you now fragment created");
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_near_you_now, container, false);

        final ListView listView = (ListView)view.findViewById(R.id.listViewUsersNearYour);
        adapter = new WListViewArrayAdapter(getActivity(), listUsers);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> myAdapter, View myView, int myItemInt, long mylng) {
                //String selectedFromList =(String) (listView.getItemAtPosition(myItemInt));

                WingManWebServiceUser wu = listUsers.get(myItemInt);
                SessionManager.viewUserId = wu.getUserId();
                Log.d("info", "clicked "+SessionManager.viewUserId);
                //progressDialog2 = ProgressDialog.show(getActivity(), "Loading...", "Getting the user profile...");
                Intent i = new Intent(getActivity(), ViewUserProfileActivity.class);
                //startActivity(i);
                startActivityForResult(i, 1);

            };
        });

        return view;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == 1) {

        }
    }

    @Override
    public void onStart() {
        Log.d("trace", "near you now fragment onStart");
        super.onStart();
    }

    @Override
    public void onResume() {
        Log.d("trace", "near you now fragment onResume");
        super.onResume();

        progressDialog = ProgressDialog.show(getActivity(), "Loading...", "Finding your matches...");
        if(null == thread1) {
            threadGetUsers = new ThreadGetUsers();
            threadGetUsers.adapter = adapter;
            threadGetUsers.context = getActivity();
            threadGetUsers.listUsers = listUsers;
            threadGetUsers.progressDialog = progressDialog;
            thread1 = new Thread(threadGetUsers);
            thread1.start();
        }
    }

    @Override
    public void onPause() {
        Log.d("trace", "near you now fragment onPause");

        super.onPause();

        if(thread1 != null) {
            threadGetUsers.stop = true;
            try {
                thread1.join();
            }catch(Exception ex){}
            thread1 = null;
        }

        Log.d("trace", "near you now fragment Paused!");
    }
}
