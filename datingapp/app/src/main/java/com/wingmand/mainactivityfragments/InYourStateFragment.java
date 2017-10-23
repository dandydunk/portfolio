package com.wingmand.mainactivityfragments;


import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.wingmand.R;
import com.wingmanlib.WingManUtils;
import com.wingmanlib.WingManWebServiceUser;

import java.util.ArrayList;


/**
 * A simple {@link Fragment} subclass.
 */
public class InYourStateFragment extends Fragment {
    ArrayList<WingManWebServiceUser> itemName = new ArrayList<WingManWebServiceUser>();
    WListViewArrayAdapter adapter;

    public InYourStateFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.d("trace", "near you now fragment created");
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_in_your_state, container, false);

        ListView listView = (ListView)view.findViewById(R.id.listViewUsersNearYour);
        adapter = new WListViewArrayAdapter(getActivity(), itemName);
        listView.setAdapter(adapter);



       /* new Thread(new Runnable() {
            @Override
            public void run() {
                ArrayList<WingManWebServiceUser> listUsers = null;

                try {
                    Log.d("info", "getting users by radius");
                    listUsers = WingManWebServiceUser.GetUsersByRadius();
                } catch(final Exception ex) {
                    Log.d("stackTrace", "exception", ex);
                    getActivity().runOnUiThread(new Runnable() {
                        public void run() {
                            WingManUtils.ShowErrorDialog(getActivity(), ex.getMessage());
                        }
                    });
                    return;
                }

                //Log.d("info", "got ["+listUsers.size()+"] users");
                for(int i =0; i < listUsers.size(); i++) {
                    //Log.d("info", "adding ["+listUsers.get(i).getFirstName()+"]");

                    final WingManWebServiceUser wsu = listUsers.get(i);
                    getActivity().runOnUiThread(new Runnable() {
                        public void run() {
                            listUsers.add(wsu);
                            adapter.notifyDataSetChanged();
                        }
                    });
                }
            }
        }).start();*/

        return view;
    }
}
