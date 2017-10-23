package com.wingmand.inboxfragments;


import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.wingmand.R;
import com.wingmand.mainactivityfragments.WListViewArrayAdapter;
import com.wingmanlib.WingManWebServiceUser;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

/**
 * A simple {@link Fragment} subclass.
 */
public class WaitingYourAuthFragment extends Fragment {
    ArrayList<WingManWebServiceUser> itemName = new ArrayList<WingManWebServiceUser>();
    InboxAuthorizationListViewArrayAdapter adapter;
    Timer timer;

    public WaitingYourAuthFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.d("info", "inbox waiting auth fragment[createView]");

        View view = inflater.inflate(R.layout.fragment_waiting_your_auth, container, false);

        //ListView listView = (ListView)view.findViewById(R.id.listViewUsersNearYour);
        //adapter = new InboxAuthorizationListViewArrayAdapter(getActivity(), listUsers);
        //listView.setAdapter(adapter);

        return view;
    }

    @Override
    public void onStart() {
        super.onStart();
        Log.d("info", "inbox waiting auth fragment[onStart]");
    }

    @Override
    public void onStop() {
        super.onStop();
        Log.d("info", "inbox waiting auth fragment[onStop]");
    }
}
