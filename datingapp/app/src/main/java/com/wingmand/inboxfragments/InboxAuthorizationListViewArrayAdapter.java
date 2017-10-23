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
import com.wingmanlib.WingManWebServiceUser;

import java.util.ArrayList;

public class InboxAuthorizationListViewArrayAdapter extends ArrayAdapter<WingManWebServiceUser> {
    ArrayList<WingManWebServiceUser> itemName;
    Activity context;

    public InboxAuthorizationListViewArrayAdapter(Activity context_, ArrayList<WingManWebServiceUser> itemName) {
        super(context_, 0, itemName);
        this.itemName = itemName;
        this.context = context_;
    }

    @Override
    public View getView (int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater=context.getLayoutInflater();
        View rowView=inflater.inflate(R.layout.inbox_authorization_listview, null,true);

        TextView textViewName = (TextView) rowView.findViewById(R.id.textViewName);
        ImageView imageView = (ImageView) rowView.findViewById(R.id.icon);

        textViewName.setText(itemName.get(position).getUserName());
        imageView.setImageBitmap(itemName.get(position).getBytesProfilePic());

        return rowView;
    }
}
