package com.wingmand.mainactivityfragments;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.content.Context;
import android.widget.ImageView;
import android.widget.TextView;

import com.wingmand.R;
import com.wingmanlib.WingManWebServiceUser;

import java.util.ArrayList;
import java.util.List;

public class WListViewArrayAdapter extends ArrayAdapter<WingManWebServiceUser> {
    ArrayList<WingManWebServiceUser> itemName;
    Activity context;

    public WListViewArrayAdapter(Activity context_, ArrayList<WingManWebServiceUser> itemName) {
        super(context_, 0, itemName);
        this.itemName = itemName;
        this.context = context_;
    }

    @Override
    public View getView (int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater=context.getLayoutInflater();
        View rowView=inflater.inflate(R.layout.w_listview, null,true);

        TextView textViewName = (TextView) rowView.findViewById(R.id.textViewName);
        ImageView imageView = (ImageView) rowView.findViewById(R.id.icon);
        TextView textViewDistance = (TextView) rowView.findViewById(R.id.textViewDistance);

        textViewName.setText(itemName.get(position).getUserName());
        imageView.setImageBitmap(itemName.get(position).getBytesProfilePic());
        textViewDistance.setText(itemName.get(position).getDistanceFromSessionUser()+" miles away");

        return rowView;
    }
}
