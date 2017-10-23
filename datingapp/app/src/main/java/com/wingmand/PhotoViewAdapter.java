package com.wingmand;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;

import com.wingmanlib.WingManPhoto;
import com.wingmanlib.WingManWebServiceUser;

import java.lang.reflect.Array;
import java.util.ArrayList;


public class PhotoViewAdapter extends ArrayAdapter<WingManPhoto> {
    ArrayList<WingManPhoto> itemName;
    Activity context;

    public PhotoViewAdapter(Activity context_, ArrayList<WingManPhoto> itemName) {
        super(context_, 0, itemName);
        this.itemName = itemName;
        this.context = context_;
    }

    @Override
    public View getView (int position, View convertView, ViewGroup parent) {
        WingManImageView imgView = new WingManImageView(context);
        WingManPhoto wmp = itemName.get(position);
        imgView.setWingManPhoto(wmp);
        imgView.setImageBitmap(wmp.getBitMapPic());
        imgView.setLayoutParams(new GridView.LayoutParams(168, 168));
        return imgView;
    }
}
