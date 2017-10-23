package com.wingmanlib;

/**
 * Created by marcus on 10/5/2016.
 */

public class WingManWebService {
    //35.163.78.46
    public static String URL = "http://35.163.78.46/wingman/webService.php";

    public static String GetUrlWithAction(String action) {
        return URL + "?action=" + action;
    }
}
