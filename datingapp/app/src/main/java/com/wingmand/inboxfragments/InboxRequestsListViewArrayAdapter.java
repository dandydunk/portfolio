package com.wingmand.inboxfragments;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.wingmand.R;
import com.wingmand.SessionManager;
import com.wingmanlib.WingManInbox;
import com.wingmanlib.WingManWebServiceInbox;

import java.util.ArrayList;

/**
 * Created by deymack on 4/17/2017.
 */

public class InboxRequestsListViewArrayAdapter extends ArrayAdapter<WingManInbox> {
    ArrayList<WingManInbox> itemName;
    Activity context;

    public InboxRequestsListViewArrayAdapter(Activity context_, ArrayList<WingManInbox> itemName) {
        super(context_, 0, itemName);
        this.itemName = itemName;
        this.context = context_;
    }

    @Override
    public View getView (final int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater=context.getLayoutInflater();
        View rowView=inflater.inflate(R.layout.inbox_requests_listview, null,true);

        TextView textViewName = (TextView) rowView.findViewById(R.id.textViewName);
        TextView textViewDateSent = (TextView) rowView.findViewById(R.id.textViewDateSent);
        ImageView imageView = (ImageView) rowView.findViewById(R.id.icon);
        Button buttonReject = (Button)rowView.findViewById(R.id.buttonDelete);
        buttonReject.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                final Button b = (Button)view;
                b.setEnabled(false);
                final WingManInbox i = itemName.get(position);
                Log.d("trace", "clicked ["+Integer.toString(i.id)+"] to delete");

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        WingManWebServiceInbox inbox = new WingManWebServiceInbox();
                        inbox.setUserSessionId(SessionManager.getSessionId());
                        try {
                            inbox.Reject(i.id);
                        } catch(Exception ex) {
                            Log.d("trace", "exception", ex);
                            return;
                        }

                        context.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                for(int ii = 0; ii < itemName.size(); ii++) {
                                    WingManInbox inbox_ = itemName.get(ii);
                                    if(inbox_.id == i.id) {
                                        Log.d("trace", "removing["+Integer.toString(i.id)+"]");
                                        itemName.remove(ii);
                                        notifyDataSetChanged();
                                        return;
                                    }
                                }
                            }
                        });
                    }
                }).start();
            }
        });


        Button buttonApprove = (Button)rowView.findViewById(R.id.buttonApprove);
        buttonApprove.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                final Button b = (Button)view;
                b.setEnabled(false);
                final WingManInbox i = itemName.get(position);
                Log.d("trace", "clicked ["+Integer.toString(i.id)+"] to approve");
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        WingManWebServiceInbox inbox = new WingManWebServiceInbox();
                        inbox.setUserSessionId(SessionManager.getSessionId());
                        try {
                            inbox.Approve(i.id);
                        } catch(Exception ex) {
                            Log.d("trace", "exception", ex);
                            return;
                        }

                        context.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                for(int ii = 0; ii < itemName.size(); ii++) {
                                    WingManInbox inbox_ = itemName.get(ii);
                                    if(inbox_.id == i.id) {
                                        Log.d("trace", "removing["+Integer.toString(i.id)+"]");
                                        itemName.remove(ii);
                                        notifyDataSetChanged();
                                        return;
                                    }
                                }
                            }
                        });
                    }
                }).start();
            }
        });

        textViewName.setText(itemName.get(position).fromUser.getUserName());
        textViewDateSent.setText(itemName.get(position).dateSent.toString());

        imageView.setImageBitmap(itemName.get(position).fromUser.getBytesProfilePic());

        return rowView;
    }
}
