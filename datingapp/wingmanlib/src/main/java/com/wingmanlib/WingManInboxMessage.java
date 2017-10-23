package com.wingmanlib;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import java.util.ArrayList;

/**
 * Created by deymack on 3/31/2017.
 */

public class WingManInboxMessage {
    public int id;
    public int toUserId;
    public String message;
    public int fromUserId;
    public WingManWebServiceUser fromUser;
    public WingManWebServiceUser recipientUser;
    public int inboxId;
    public String dateSent;

    public void Save(Context context) {
        WingManDB wdb = new WingManDB(context);
        SQLiteDatabase sdb= wdb.getReadableDatabase();

        ContentValues cv = new ContentValues();
        cv.put("id", this.id);
        cv.put("message", this.message);
        cv.put("dateSent", this.dateSent);
        cv.put("fromUserId", this.fromUserId);
        cv.put("inboxId", this.inboxId);
        cv.put("recipientUserId", this.recipientUser.getUserId());
        cv.put("toUserId", this.toUserId);

        sdb.insert("inboxMessages", null, cv);

        sdb.close();
    }

    static public ArrayList<WingManInboxMessage> Get(Context context, int lastId, int userId) throws Exception {
        WingManDB db = new WingManDB(context);
        SQLiteDatabase sDb = db.getReadableDatabase();
        Cursor c = sDb.query(true, "inboxMessages",
                                new String[]{"id", "message", "dateSent", "fromUserId", "inboxId", "recipientUserId"},
                                "id > "+lastId+" AND toUserId="+userId, null, "inboxId", null, "id DESC", null);
        ArrayList<WingManInboxMessage> listM = new ArrayList<WingManInboxMessage>();
        while(c.moveToNext()) {


            WingManInboxMessage m = new WingManInboxMessage();
            m.id = c.getInt(0);
            m.message = c.getString(1);
            m.dateSent = c.getString(2);
            m.fromUserId = c.getInt(3);
            m.inboxId = c.getInt(4);

            WingManWebServiceUser user = new WingManWebServiceUser();
            user.setUserId(m.fromUserId);
            user.GetProfileById();

            m.fromUser = user;

            WingManWebServiceUser userR = new WingManWebServiceUser();
            userR.setUserId(c.getInt(5));
            userR.GetProfileById();

            m.recipientUser = userR;

            listM.add(m);
        }

        sDb.close();

        return listM;
    }

    static public ArrayList<WingManInboxMessage> GetByInboxId(Context context, int inboxId, int lastId) throws Exception {
        WingManDB db = new WingManDB(context);
        SQLiteDatabase sDb = db.getReadableDatabase();
        Cursor c = sDb.query("inboxMessages",
                new String[]{"id", "message", "dateSent", "fromUserId", "inboxId", "recipientUserId"},
                "id > "+lastId+" AND inboxId="+inboxId, null, null, null, null);

        ArrayList<WingManInboxMessage> listM = new ArrayList<WingManInboxMessage>();
        while(c.moveToNext()) {
            WingManInboxMessage m = new WingManInboxMessage();
            m.id = c.getInt(0);
            m.message = c.getString(1);
            m.dateSent = c.getString(2);
            m.fromUserId = c.getInt(3);
            m.inboxId = c.getInt(4);

            /*WingManWebServiceUser user = new WingManWebServiceUser();
            user.setUserId(m.fromUserId);
            user.GetProfileById();

            m.fromUser = user;

            WingManWebServiceUser userR = new WingManWebServiceUser();
            userR.setUserId(c.getInt(5));
            userR.GetProfileById();

            m.recipientUser = userR;*/

            listM.add(m);
        }

        sDb.close();

        return listM;
    }
}
