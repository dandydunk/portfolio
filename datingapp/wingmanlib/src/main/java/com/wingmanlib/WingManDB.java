package com.wingmanlib;

import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteDatabase;
import android.content.Context;

public class WingManDB extends SQLiteOpenHelper {
    public static final int DATABASE_VERSION = 4;
    public static final String DATABASE_NAME = "WingManDB.db";

    public WingManDB(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    public void onCreate(SQLiteDatabase db) {
        db.beginTransaction();
        String query = "CREATE TABLE `settings` (`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, `name` TEXT NOT NULL UNIQUE, `value`	TEXT )";
        db.execSQL(query);

        query = "CREATE TABLE `inboxmessages` (`id` INTEGER NOT NULL,`message` TEXT NOT NULL,`fromUserId` INTEGER NOT NULL, `toUserId` INTEGER NOT NULL, `dateSent` TEXT NOT NULL,`inboxId` INTEGER NOT NULL, `recipientUserId` INTEGER)";
        db.execSQL(query);

        db.setTransactionSuccessful();
        db.endTransaction();
    }

    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // This database is only a cache for online data, so its upgrade policy is
        // to simply to discard the data and start over
        String query = "DROP TABLE IF EXISTS settings";
        db.execSQL(query);

        query = "DROP TABLE IF EXISTS inboxMessages";
        db.execSQL(query);

        onCreate(db);
    }
}
