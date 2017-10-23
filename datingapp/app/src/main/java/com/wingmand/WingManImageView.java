package com.wingmand;

import android.content.Context;
import android.widget.ImageView;

import com.wingmanlib.WingManPhoto;

public class WingManImageView extends ImageView {
    public WingManImageView(Context context){
        super(context);
    }

    public WingManPhoto getWingManPhoto() {
        return wingManPhoto;
    }

    public void setWingManPhoto(WingManPhoto wingManPhoto) {
        this.wingManPhoto = wingManPhoto;
    }

    WingManPhoto wingManPhoto;
}
