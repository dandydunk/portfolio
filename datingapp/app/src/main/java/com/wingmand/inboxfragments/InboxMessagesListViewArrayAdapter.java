package com.wingmand.inboxfragments;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.wingmand.R;
import com.wingmand.SessionManager;
import com.wingmanlib.InboxMessagePreview;
import com.wingmanlib.WingManInboxMessage;

import java.util.ArrayList;


public class InboxMessagesListViewArrayAdapter extends ArrayAdapter<WingManInboxMessage> {
    ArrayList<WingManInboxMessage> itemName;
    Activity context;

    public InboxMessagesListViewArrayAdapter(Activity context_, ArrayList<WingManInboxMessage> itemName) {
        super(context_, 0, itemName);
        this.itemName = itemName;
        this.context = context_;
    }

    @Override
    public View getView (int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater=context.getLayoutInflater();
        View rowView=inflater.inflate(R.layout.inbox_messages_listview, null,true);

        TextView textViewName = (TextView) rowView.findViewById(R.id.textViewName);
        TextView textViewMessage= (TextView) rowView.findViewById(R.id.textViewMessage);
        TextView textViewDateSent = (TextView) rowView.findViewById(R.id.textViewDateSent);
        ImageView imageView = (ImageView) rowView.findViewById(R.id.icon);

        textViewName.setText(itemName.get(position).recipientUser.getUserName());
        if(SessionManager.getUserId() == itemName.get(position).fromUserId) {
            textViewMessage.setText("From You: "+itemName.get(position).message);
        }
        else {
            textViewMessage.setText(itemName.get(position).message);
        }
        textViewDateSent.setText(itemName.get(position).dateSent.toString());

        imageView.setImageBitmap(itemName.get(position).recipientUser.getBytesProfilePic());

        return rowView;
    }
}
