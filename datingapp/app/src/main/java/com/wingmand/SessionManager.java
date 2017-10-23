package com.wingmand;


import android.location.Location;

public class SessionManager {
    public static int getUserId() {
        return userId;
    }

    public static void setUserId(int userId) {
        SessionManager.userId = userId;
    }

    public static int userId = 0;

    public static int viewInboxId = 0;

    public static String getSessionId() {
        return sessionId;
    }

    public static void setSessionId(String sessionId) {
        SessionManager.sessionId = sessionId;
    }

    public static boolean isLoggedIn() {
        return sessionId == null ? false:true;
    }

    public static String sessionId = null;
    public static boolean appStarted = false;
    public static int viewUserId = 0;

    public static class GPS {
        public enum GPSStatus {
            NOT_CONNECTED,
            CONNECTING,
            GOT_LOCATION,
            QUERYING_LOCATION
        }
        public static Location lastLocation = null;
        public static GPSStatus status = GPSStatus.NOT_CONNECTED;
    }
}
