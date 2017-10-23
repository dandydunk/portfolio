package com.wingmanlib;

/**
 * Created by marcus on 10/4/2016.
 */

import android.app.AlertDialog;
import android.content.Context;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;

public class WingManUtils {
    public static void ShowErrorDialog(Context context, String msg) {
        AlertDialog.Builder builder1 = new AlertDialog.Builder(context);
        builder1.setMessage("MSG: "+msg);
        builder1.setTitle("ERROR");
        builder1.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        AlertDialog alert11 = builder1.create();
        alert11.show();
    }

    public static void ShowSuccessDialog(Context context, String msg) {
        AlertDialog.Builder builder1 = new AlertDialog.Builder(context);
        builder1.setMessage(msg);
        builder1.setTitle("SUCCESS");
        builder1.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        AlertDialog alert11 = builder1.create();
        alert11.show();
    }
}
