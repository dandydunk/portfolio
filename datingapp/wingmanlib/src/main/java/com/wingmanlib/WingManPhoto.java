package com.wingmanlib;


import android.graphics.Bitmap;

public class WingManPhoto {
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public Bitmap getBitMapPic() {
        return bitMapPic;
    }

    public void setBitMapPic(Bitmap bitMapPic) {
        this.bitMapPic = bitMapPic;
    }

    Bitmap bitMapPic;
    int id;
}
