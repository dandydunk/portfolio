package com.wingmanlib;

import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteStatement;
import android.content.Context;
import android.database.Cursor;

public class WingManSettings {
    String sessionId = null;
    Context context = null;

    public WingManSettings(Context _context) {
        context = _context;
    }

    public String GetByName(String name) {
        WingManDB db = new WingManDB(context);
        SQLiteDatabase sDb = db.getReadableDatabase();
        Cursor c = sDb.rawQuery("SELECT value FROM settings WHERE name=?", new String[]{name});
        if(!c.moveToNext()) {
            return null;
        }

        String value = c.getString(0);
        sDb.close();
        c.close();

        return value;
    }

    public void UpdateSetting(String name, String value) {
        WingManDB wdb = new WingManDB(context);
        SQLiteDatabase sdb= wdb.getReadableDatabase();

        if(null == GetByName(name)) {
            sdb.execSQL("INSERT INTO settings(name, value) VALUES(?, ?)", new String[]{name, value});
        }
        else {
            sdb.execSQL("UPDATE settings SET value=? WHERE name=?", new String[]{value, name});
        }

        sdb.close();
    }
}
