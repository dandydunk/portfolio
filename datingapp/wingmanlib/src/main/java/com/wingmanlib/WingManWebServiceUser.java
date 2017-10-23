package com.wingmanlib;

import java.io.IOException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashMap;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.location.Location;
import android.util.Log;
import android.util.Base64;
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONTokener;

import static android.util.Base64.DEFAULT;

public class WingManWebServiceUser extends WingManWebService {
    public String getSessionId() {
        return sessionId;
    }

    public void setSessionId(String sessionId) {
        this.sessionId = sessionId;
    }

    public double getDistanceFromSessionUser() {
        return distanceFromSessionUser;
    }

    public void setDistanceFromSessionUser(double distanceFromSessionUser) {
        this.distanceFromSessionUser = distanceFromSessionUser;
    }

    double distanceFromSessionUser = 0;


    public ArrayList<WingManWebServiceUser> GetUsersByRadius() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.sessionId);
        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getUsersByRadius"), mapParams);
        Object json = new JSONTokener(httpResponse).nextValue();
        ArrayList<WingManWebServiceUser> listWu = new ArrayList<WingManWebServiceUser>();
        if (json instanceof JSONObject) {
            JSONObject jo = new JSONObject(httpResponse);
            if (jo.has("error")) {
                throw new WingManException(jo.getString("error"));
            }
        }

        else if (json instanceof JSONArray) {
            JSONArray ja = new JSONArray(httpResponse);
            int jaLen = ja.length();

            for (int i = 0; i < jaLen; i++) {
                JSONObject jo = ja.getJSONObject(i);
                WingManWebServiceUser wu = new WingManWebServiceUser();
                wu.setUserId(jo.getInt("userId"));
                wu.GetProfileById();
                wu.setDistanceFromSessionUser(jo.getDouble("distanceFromSessionUser"));
                listWu.add(wu);
            }
        }

        return listWu;
    }


    public void GetProfileById() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("userId", Integer.toString(this.userId));

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getUserProfile"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

        if(!jo.has("userName")) {
            throw new WingManException("the user name was missing.");
        }

        if(!jo.has("profilePic")) {
            throw new WingManException("the profile pic was missing.");
        }

        this.userName = jo.getString("userName");
        byte[] b = Base64.decode(jo.getString("profilePic"), DEFAULT);
        this.bytesProfilePic = BitmapFactory.decodeByteArray(b, 0, b.length);
        //Log.d("info", "first name="+this.firstName);
    }

    public WingManLocation GetLocation() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.sessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getLocation"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if (jo.has("error")) {
            if(jo.has("errorNumber")) {
                int errNum = jo.getInt("errorNumber");
                if(errNum == 102) {
                    return null;
                }
            }
            throw new WingManException(jo.getString("error"));
        }

        WingManLocation l = new WingManLocation();
        l.address = jo.getString("address");
        l.city = jo.getString("city");
        l.state = jo.getString("state");
        l.latitude = jo.getDouble("latitude");
        l.longitude = jo.getDouble("longitude");
        l.place = jo.getString("place");
        l.state = jo.getString("state");
        l.postalCode = jo.getString("postalCode");
        l.street = jo.getString("street");

        return l;
    }

    public void GetInbox(int userId) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("userId", Integer.toString(this.userId));

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getUserProfile"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

    }

    public ArrayList<WingManPhoto> GetPhotos() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.sessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getPhotosBySession"), mapParams);
        Object json = new JSONTokener(httpResponse).nextValue();

        ArrayList<WingManPhoto> listPhotos = new ArrayList<WingManPhoto>();
        if (json instanceof JSONObject) {
            JSONObject jo = new JSONObject(httpResponse);
            if (jo.has("error")) {
                throw new WingManException(jo.getString("error"));
            }
        }

        else if (json instanceof JSONArray) {
            JSONArray ja = new JSONArray(httpResponse);
            int jaLen = ja.length();

            for (int i = 0; i < jaLen; i++) {
                JSONObject jo = ja.getJSONObject(i);
                byte[] photoBytes = Base64.decode(jo.getString("picBytes"), DEFAULT);

                WingManPhoto wmp = new WingManPhoto();
                wmp.setId(jo.getInt("id"));
                wmp.setBitMapPic(BitmapFactory.decodeByteArray(photoBytes, 0, photoBytes.length));
                listPhotos.add(wmp);
            }
        }

        return listPhotos;
    }

    public void DeletePhoto(int photoId) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.sessionId);
        mapParams.put("photoId", Integer.toString(photoId));

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("deletePhoto"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public void GetProfile() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.sessionId);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("getProfileValues"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

        if(!jo.has("userName")) {
            throw new WingManException("the first name was missing.");
        }

        if(!jo.has("birthday")) {
            throw new WingManException("the birthday was missing.");
        }

        this.userName = jo.getString("userName");
        this.birthday = jo.getString("birthday");
    }

    public void UpdateProfile() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.sessionId);
        mapParams.put("userName", this.userName);
        mapParams.put("birthday", this.birthday);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("updateProfile"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public void UploadPhoto(String photoBytes) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("sessionId", this.sessionId);
        mapParams.put("photoBytes", photoBytes);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("uploadPhoto"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public void CheckIn(Location location, String placeName) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();

        mapParams.put("sessionId", this.sessionId);
        mapParams.put("latitude", Double.toString(location.getLatitude()));
        mapParams.put("longitude", Double.toString(location.getLongitude()));
        mapParams.put("placeName", URLEncoder.encode(placeName, "UTF-8"));

        String httpResponse = null;
        try {
            httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("saveCheckIn"), mapParams);
        } catch(IOException ex) {
            Log.d("error", "post error ="+ex.getMessage());
            throw ex;
        }
        Log.d("stacktrace", "check in response["+httpResponse+"]");
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }
    }

    public void NewAccount() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("email", this.email);
        mapParams.put("password", this.password);
        mapParams.put("passwordConfirm", this.passwordConfirm);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("newAccount"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

        if(!jo.has("sessionId")) {
            throw new WingManException("the session id was missing.");
        }

        this.sessionId = jo.getString("sessionId");
    }

    public void Login() throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("email", this.email);
        mapParams.put("password", this.password);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("login"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

        if(!jo.has("sessionId")) {
            throw new WingManException("the session id was missing.");
        }

        if(!jo.has("userId")) {
            throw new WingManException("the user id was missing.");
        }

        this.sessionId = jo.getString("sessionId");
        this.userId = jo.getInt("userId");
    }

    public void FBLogin(String accessToken) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("accessToken", accessToken);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("fbLogin"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

        if(!jo.has("sessionId")) {
            throw new WingManException("the session id was missing.");
        }

        if(!jo.has("userId")) {
            throw new WingManException("the user id was missing.");
        }

        this.sessionId = jo.getString("sessionId");
        this.userId = jo.getInt("userId");
    }

    public void FBNewAccount(String accessToken) throws IOException, org.json.JSONException, WingManException {
        HashMap<String, String> mapParams = new HashMap<String, String>();
        mapParams.put("accessToken", accessToken);

        String httpResponse = WingManInternet.SendHTTPPost(GetUrlWithAction("fbNewAccount"), mapParams);
        JSONObject jo = new JSONObject(httpResponse);
        if(jo.has("error")) {
            throw new WingManException(jo.getString("error"));
        }

        if(!jo.has("sessionId")) {
            throw new WingManException("the session id was missing.");
        }

        if(!jo.has("userId")) {
            throw new WingManException("the user id was missing.");
        }

        this.sessionId = jo.getString("sessionId");
        this.userId = jo.getInt("userId");
    }

    public String getPasswordConfirm() {
        return passwordConfirm;
    }

    public void setPasswordConfirm(String passwordConfirm) {
        this.passwordConfirm = passwordConfirm;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    private String email;
    private String password;
    private String passwordConfirm;
    private String sessionId;

    public String getUserName() {
        return userName;
    }

    public void setUserName(String userName) {
        this.userName = userName;
    }

    private String userName;

    public String getBirthday() {
        return birthday;
    }

    public void setBirthday(String birthday) {
        this.birthday = birthday;
    }

    String birthday;

    public Bitmap getBytesProfilePic() {
        return bytesProfilePic;
    }

    public void setBytesProfilePic(Bitmap bytesProfilePic) {
        this.bytesProfilePic = bytesProfilePic;
    }

    Bitmap bytesProfilePic;

    public int getUserId() {
        return userId;
    }

    public void setUserId(int userId) {
        this.userId = userId;
    }

    int userId;

    public float getDistance() {
        return distance;
    }

    public void setDistance(float distance) {
        this.distance = distance;
    }

    float distance;
}
